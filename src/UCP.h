#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId);
	~UCP();

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

public:
	// Negotiation control -------------------------

	void StartUCCNegotiation(const TCPSocketPtr& socket, uint16_t negotiating_ucc_id);

	// ---------------------------------------------

	// Communication control -----------------------



	// ---------------------------------------------

	// Spawning control ----------------------------



	// ---------------------------------------------

private:
	uint16_t negotiating_ucc_id = 0;
};

