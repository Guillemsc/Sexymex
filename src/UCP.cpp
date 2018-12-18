#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"
#include "AgentLocation.h"

// TODO: Make an enum with the states
enum State
{
	ST_IDLE,

	ST_WAITING_UCC_NEGOTIATION_RESPONSE,
	ST_NEGOTIATING,

	ST_NEGOTIATION_FINISHED,
};

UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId) : 
	_requestedItemId(requestedItemId),
	_contributedItemId(contributedItemId),
	Agent(node)
{
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
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::MCCToMCPNegotiationResponse:
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

			}
			else
			{

			}
		}
	}
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
