#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"
#include "AgentLocation.h"
#include "ModuleNodeCluster.h"

// TODO: Make an enum with the states
enum State
{
	ST_IDLE,

	ST_WAITING_UCC_NEGOTIATION_RESPONSE,

	ST_NEGOTIATION_FINISHED,
};

UCP::UCP(Node *node, MCP* _mcp, uint16_t requestedItemId, uint16_t contributedItemId) :
	_requestedItemId(requestedItemId),
	_contributedItemId(contributedItemId),
	Agent(node)
{
	this->parent_mcp = _mcp;
	this->negotiating_ucc_id = negotiating_ucc_id;

	setState(State::ST_IDLE);
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
		// TODO: Handle states

	default:;
	}
}

void UCP::stop()
{
	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::UCCToUCPNegotiationResponse:
	{
		if (state() == State::ST_WAITING_UCC_NEGOTIATION_RESPONSE)
		{
			PacketUCCToUCPNegotiationResponse packetData;
			packetData.Deserialize(stream);

			HandleUCCNegotiationResponse(socket, packetData.response, packetData.solution_found, packetData.UCCconstraint);
		}

		break;
	}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::StartUCCNegotiation(const AgentLocation& agent, uint16_t negotiating_ucc_id)
{
	if (state() == State::ST_IDLE)
	{
		this->negotiating_ucc_id = negotiating_ucc_id;

		StartNegotation_SendToUCC(agent);

		setState(State::ST_WAITING_UCC_NEGOTIATION_RESPONSE);
	}
}

void UCP::HandleUCCNegotiationResponse(const TCPSocketPtr & socket, bool response, bool solution_found, uint16_t ucc_constraint)
{
	if (state() == State::ST_WAITING_UCC_NEGOTIATION_RESPONSE)
	{
		if (response)
		{
			if (solution_found)
			{
				parent_mcp->ChildUCPSolutionFound();

				iLog << "UCP with [" << _requestedItemId << " : " << _contributedItemId << "] started negotiation and found solution";
			}
			else if (parent_mcp->GetDeepnees() <= 5)
			{
				parent_mcp->createChildMCP(ucc_constraint);

				iLog << "UCP with ["<< _requestedItemId << " : " << _contributedItemId << "] started negotiation with item [" << _requestedItemId << " : " << ucc_constraint << "] but not found solution, creating new MCP [" << ucc_constraint << " : " << _contributedItemId << "]";
			}
			else
			{
				iLog << "UCP with [" << _requestedItemId << " : " << _contributedItemId << "] started negotiation with item [" << _requestedItemId << " : " << ucc_constraint << "] but not found solution, we don't create another child MCP since deepness would be > 5";

				parent_mcp->ChildUCPNegotiationNotFound();
			}
		}
		else
		{
			parent_mcp->ChildUCPNegotiationNotFound();
		}
	}

	setState(State::ST_NEGOTIATION_FINISHED);
	stop();
}

bool UCP::StartNegotation_SendToUCC(const AgentLocation & agent)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UCPToUCCNegotiationRequest;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = negotiating_ucc_id;

	PacketUCPToUCCNegotiationRequest packetData;
	packetData.ucp_offer = _contributedItemId;
	packetData.ucp_request = _requestedItemId;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	return sendPacketToAgent(agent.hostIP, agent.hostPort, stream);
}
