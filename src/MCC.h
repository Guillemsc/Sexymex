#pragma once
#include "Agent.h"

// Forward declaration
class UCC;
using UCCPtr = std::shared_ptr<UCC>;

class MCC :
	public Agent
{
public:

	// Constructor and destructor
	MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);
	~MCC();

	// Agent methods
	void update() override;
	void stop() override;
	MCC* asMCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	// Getters
	bool isIdling() const;
	uint16_t contributedItemId() const { return _contributedItemId; }
	uint16_t constraintItemId() const { return _constraintItemId; }

	// Whether or not the negotiation finished
	bool negotiationFinished() const;

	// Whether or not there was a negotiation agreement
	bool negotiationAgreement() const;

private:
	// Negotiation control -------------------------

	void HandleMCPNegotiationRequest(const TCPSocketPtr& socket, uint16_t mcp_id, uint16_t mcp_offer, uint16_t mcp_request);

	// ---------------------------------------------

	// Communication control -----------------------

	bool Register_SendToYellowPages();
	void Unregister_SendToYellowPages();

	void NegotiationResponse_SendToMCP(const TCPSocketPtr& socket, uint16_t mpc_id, bool response, uint16_t ucc_id = 0);

	// ---------------------------------------------

	// Spawning control ----------------------------

	void createChildUCC();

	void destroyChildUCC();

	// ---------------------------------------------

private:

	uint16_t _contributedItemId; /**< The contributed item. */
	uint16_t _constraintItemId; /**< The constraint item. */

	UCCPtr _ucc; /**< Child UCC. */
};
