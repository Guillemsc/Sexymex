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

	bool registerIntoYellowPages();
	
	void unregisterFromYellowPages();

	void createChildUCC();

	void destroyChildUCC();

	uint16_t _contributedItemId; /**< The contributed item. */
	uint16_t _constraintItemId; /**< The constraint item. */

	UCCPtr _ucc; /**< Child UCC. */
};
