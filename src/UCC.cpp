#include "UCC.h"

// TODO: Make an enum with the states
enum State
{
	ST_IDLE,

	ST_WAITING_UCC_NEGOTIATION_RESPONSE,
	ST_NEGOTIATING,

	ST_NEGOTIATION_FINISHED,
};

UCC::UCC(Node *node, MCC* _mcc, uint16_t contributedItemId, uint16_t constraintItemId) :
	_constraintItemId(constraintItemId),
	_contributedItemId(contributedItemId),
	Agent(node)
{
	this->_mcc = _mcc;

	setState(State::ST_IDLE);
}

UCC::~UCC()
{
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::UCPToUCCNegotiationRequest:
	{		
		PacketUCPToUCCNegotiationRequest packetData;
		packetData.Deserialize(stream);

		HandleUCPNegotiationRequest(socket, packetHeader.srcAgentId, packetData.ucp_offer, packetData.ucp_request);
		
		break;
	}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCC::HandleUCPNegotiationRequest(const TCPSocketPtr& socket, uint16_t ucp_id, uint16_t ucp_offer, uint16_t ucp_request)
{
	bool negotiate = false;
	bool solution_found = false;

	if (ucp_request == _contributedItemId && state() == State::ST_IDLE)
	{
		negotiate = true;

		if (ucp_offer == _constraintItemId)
		{
			solution_found = true;
		}
	}

	NegotiationResponse_SendToUCP(socket, ucp_id, negotiate, solution_found);

	setState(State::ST_NEGOTIATION_FINISHED);
	stop();
}

void UCC::NegotiationResponse_SendToUCP(const TCPSocketPtr & socket, uint16_t ucp_id, bool response, bool solution_found)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UCCToUCPNegotiationResponse;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = ucp_id;

	PacketUCCToUCPNegotiationResponse packetData;
	packetData.response = response;
	packetData.solution_found = solution_found;
	packetData.UCCconstraint = _constraintItemId;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	socket->SendPacket(stream);
}
