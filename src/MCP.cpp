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

	ST_NEGOTIATION_FINISHED,
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth) :
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
		GetMCCsWithItem();

		break;
	}

	default:
		break;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)

	destroy();
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
	return false; // TODO: Did the child UCP find a solution?
}

void MCP::GetMCCsWithItem()
{
	GetMCCsWithItem_SendToYellowPages(_requestedItemId);

	setState(ST_REQUESTING_MCCs);
}

void MCP::InitMCCsNegotiationList(std::vector<AgentLocation> agents)
{
	_mccRegisters.swap(agents);

	_mccRegisterIndex = 0;
	setState(ST_ITERATING_OVER_MCCs);

	StartCurrentMCCNegotiation();
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

			_ucp->StartUCCNegotiation(socket, ucc_id);
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
	packetHead.packetType = PacketType::MCPToMCCNegotiationInfo;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = mcc.agentId;

	PacketMCPToMCCNegotiationInfo packetData;
	packetData.mcp_offer = _contributedItemId;
	packetData.mcp_request = _requestedItemId;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}

void MCP::createChildUCP()
{
	destroyChildUCP();

	_ucp = App->modNodeCluster->spawnUCP(this);
}

void MCP::destroyChildUCP()
{
	if (UCPExists())
	{
		_ucp->stop();

		_ucp.reset();
	}
}

bool MCP::UCPExists()
{
	return _ucp.get();
}
