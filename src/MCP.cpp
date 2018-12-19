#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"
#include "ModuleNodeCluster.h"

enum State
{
	ST_INIT,

	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,

	ST_WAITING_MCC_NEGOTIATION_RESPONSE,
	ST_NEGOTIATING,

	ST_WATING_FOR_CHILDS_NEGOTIATIONS,

	ST_WAITING_FOR_MCC_CONNECTION_FINISH,

	ST_NEGOTIATION_FINISHED,
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID)
{
	setState(ST_INIT);
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
	{
		negotiation_agreement = false;

		GetMCCsWithItem();

		break;
	}

	default:
		break;
	}
}

void MCP::stop()
{
	destroyChildUCP();

	destroy();

	wLog << "MCP Destroyed";
}

void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
	{
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Deserialize(stream);

			// Store the returned MCCs from YP
			// Select the first MCC to negociate
			InitMCCsNegotiationList(packetData.mccAddresses);
		}

		break;
	}

	case PacketType::MCCToMCPNegotiationResponse:
	{
		if (state() == State::ST_WAITING_MCC_NEGOTIATION_RESPONSE)
		{
			PacketMCCToMCPNegotiationResponse packetData;
			packetData.Deserialize(stream);

			HandleMCCNegotiationResponse(socket, packetData.response, packetData.UCCId);
		}

		break;
	}

	case PacketType::MCCToMCPConnectionFinished:
	{
		negotiation_agreement = true;

		if (!HasParent())
		{
			App->modNodeCluster->AddNodeOperation(node(), NodeOperationType::ADD, _requestedItemId);
			App->modNodeCluster->AddNodeOperation(node(), NodeOperationType::REMOVE, _contributedItemId);

			iLog << "MCP exchange at Node " << node()->id() << ":"
				<< " -" << contributedItemId()
				<< " +" << requestedItemId();
		}

		if (parent_mcp != nullptr)
		{
			parent_mcp->ChildMCPSolutionFound();
		}

		setState(State::ST_NEGOTIATION_FINISHED);

		break;
	}

	// TODO: Handle other packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::negotiationFinished() const
{
	return state() == ST_NEGOTIATION_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	return negotiation_agreement; // TODO: Did the child UCP find a solution?
}

void MCP::SetParent(MCP * parent)
{
	this->parent_mcp = parent;

	if(parent != nullptr)
		is_child = true;
}

bool MCP::HasParent()
{
	return parent_mcp != nullptr;
}

bool MCP::IsChild()
{
	return is_child;
}

void MCP::SetNegotiationFinishedState()
{
	setState(State::ST_NEGOTIATION_FINISHED);
}

void MCP::GetMCCsWithItem()
{
	GetMCCsWithItem_SendToYellowPages(_requestedItemId);

	setState(ST_REQUESTING_MCCs);
}

void MCP::InitMCCsNegotiationList(std::vector<AgentLocation> agents)
{
	if (state() == State::ST_REQUESTING_MCCs)
	{
		if (_requestedItemId == 8 && _contributedItemId == 7)
		{
			int i = 0;
		}

		if (agents.size() > 0)
		{
			_mccRegisters.swap(agents);

			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			StartCurrentMCCNegotiation();
		}
		else
		{
			setState(State::ST_NEGOTIATION_FINISHED);

			negotiation_agreement = false;

			if (parent_mcp != nullptr)
			{
				parent_mcp->ChildMCPSolutionNotFound();
			}

			iLog << "MCP list of MCC's is empty, finishing negotiations";
		}
	}
}

void MCP::StartCurrentMCCNegotiation()
{
	if (state() == State::ST_ITERATING_OVER_MCCs)
	{
		// We start negotiation
		if (_mccRegisterIndex < _mccRegisters.size())
		{
			AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];

			StartNegotation_SendToMCC(curr_agent);

			setState(State::ST_WAITING_MCC_NEGOTIATION_RESPONSE);
		}

		// There is a problem, finish negotiation
		else
		{
			setState(State::ST_NEGOTIATION_FINISHED);

			negotiation_agreement = false;

			if (parent_mcp != nullptr)
			{
				parent_mcp->ChildMCPSolutionNotFound();
			}
		}
	}
}

void MCP::SetNextMCC()
{
	if (state() == State::ST_ITERATING_OVER_MCCs)
	{
		// We set the next mcc to negotiate
		if (_mccRegisterIndex + 1 < _mccRegisters.size())
		{
			++_mccRegisterIndex;
		}

		// There are no mor mccs to negotiate
		else
		{
			setState(State::ST_NEGOTIATION_FINISHED);

			negotiation_agreement = false;

			if (parent_mcp != nullptr)
			{
				parent_mcp->ChildMCPSolutionNotFound();
			}

			wLog << "MCP runned out of mccs to check, finishing all negotiations";
		}
	}
}

void MCP::HandleMCCNegotiationResponse(const TCPSocketPtr& socket, bool response, uint16_t ucc_id)
{
	if (state() == State::ST_WAITING_MCC_NEGOTIATION_RESPONSE)
	{
		// MCC wants to negotiate and returns us the ucc id
		if (response)
		{
			setState(State::ST_NEGOTIATING);

			createChildUCP();

			AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];

			child_ucp->StartUCCNegotiation(curr_agent, ucc_id);
		}

		// MCC does not want to negotiate, we keep iterating
		else
		{
			setState(State::ST_ITERATING_OVER_MCCs);

			SetNextMCC();
			StartCurrentMCCNegotiation();
		}
	}
}

void MCP::ChildUCPSolutionFound()
{
	if (state() == State::ST_NEGOTIATING)
	{
		AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];
		FinishNegotiation_SendToMCC(curr_agent, true);

		setState(State::ST_WAITING_FOR_MCC_CONNECTION_FINISH);
	}
}

void MCP::ChildUCPNegotiationNotFound()
{
	if (state() == State::ST_NEGOTIATING)
	{
		AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];
		FinishNegotiation_SendToMCC(curr_agent, false);

		setState(State::ST_ITERATING_OVER_MCCs);

		destroyChildUCP();

		SetNextMCC();
		StartCurrentMCCNegotiation();
	}
}

void MCP::ChildMCPSolutionFound()
{
	if (state() == State::ST_NEGOTIATING)
	{
		AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];
		FinishNegotiation_SendToMCC(curr_agent, true);

		setState(State::ST_WAITING_FOR_MCC_CONNECTION_FINISH);
	}
}

void MCP::ChildMCPSolutionNotFound()
{
	if (state() == State::ST_NEGOTIATING)
	{
		AgentLocation curr_agent = _mccRegisters[_mccRegisterIndex];
		FinishNegotiation_SendToMCC(curr_agent, false);

		setState(State::ST_ITERATING_OVER_MCCs);

		destroyChildUCP();

		SetNextMCC();
		StartCurrentMCCNegotiation();
	}
}

bool MCP::GetMCCsWithItem_SendToYellowPages(int itemId)
{	
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;

	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}

bool MCP::StartNegotation_SendToMCC(const AgentLocation& mcc)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::MCPToMCCNegotiationRequest;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = mcc.agentId;

	PacketMCPToMCCNegotiationRequest packetData;
	packetData.mcp_offer = _contributedItemId;
	packetData.mcp_request = _requestedItemId;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}

bool MCP::FinishNegotiation_SendToMCC(const AgentLocation & mcc, bool succes)
{
	PacketHeader packetHead;
	packetHead.packetType = PacketType::MCPToMCCNegotiationFinish;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = mcc.agentId;

	PacketMCPToMCCNegotiationFinish packetData;
	packetData.succes = succes;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}

void MCP::createChildUCP()
{
	destroyChildUCP();

	child_ucp = App->modNodeCluster->spawnUCP(this).get();
}

void MCP::destroyChildUCP()
{
	if (ChildUCPExists())
	{
		child_ucp = nullptr;
	}
}

bool MCP::ChildUCPExists()
{
	return child_ucp != nullptr;
}

void MCP::createChildMCP(uint16_t requestedItemId)
{
	destroyChildMCP();

	child_mcp = App->modNodeCluster->spawnMCP(node()->id(), requestedItemId, _contributedItemId);
	child_mcp->SetParent(this);
}

void MCP::destroyChildMCP()
{
	child_mcp = nullptr;
}

bool MCP::ChildMCPExists()
{
	return child_mcp != nullptr;
}
