#pragma once
#include "Agent.h"

class UCC :
	public Agent
{
public:

	// Constructor and destructor
	UCC(Node *node, MCC* _mcc, uint16_t contributedItemId, uint16_t constraintItemId);
	~UCC();

	// Agent methods
	void update() override { }
	void stop() override;
	UCC* asUCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

public:
	// Negotiation control -------------------------

	void HandleUCPNegotiationRequest(const TCPSocketPtr& socket, uint16_t ucp_id, uint16_t ucp_offer, uint16_t ucp_request);

	// ---------------------------------------------

private:
	// Communication control -----------------------

	void NegotiationResponse_SendToUCP(const TCPSocketPtr& socket, uint16_t ucp_id, bool response, bool solution_found);

	// ---------------------------------------------

	// Spawning control ----------------------------



	// ---------------------------------------------

private:
	uint16_t _contributedItemId;
	uint16_t _constraintItemId;

	MCC* _mcc = nullptr; /**< Parent MCC. */
};

