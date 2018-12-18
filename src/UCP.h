#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;
class AgentLocation;

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

	void StartUCCNegotiation(const AgentLocation& agent, uint16_t negotiating_ucc_id);
	void HandleUCCNegotiationResponse(const TCPSocketPtr& socket, bool response, bool solution_found, uint16_t ucc_constraint);

	// ---------------------------------------------

	// Communication control -----------------------

	bool StartNegotation_SendToUCC(const AgentLocation& mcc);

	// ---------------------------------------------

	// Spawning control ----------------------------



	// ---------------------------------------------

private:
	uint16_t negotiating_ucc_id = 0;

	uint16_t _requestedItemId;
	uint16_t _contributedItemId;

	MCP* _mcp = nullptr; /**< Parent MCP. */
};

