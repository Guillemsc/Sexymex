#pragma once
#include "Agent.h"

// Forward declaration
class UCP;
using UCPPtr = std::shared_ptr<UCP>;

class MCP :
	public Agent
{
public:

	// Constructor and destructor
	MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID);
	~MCP();

	// Agent methods
	void update() override;
	void stop() override;
	MCP* asMCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	// Getters
	uint16_t requestedItemId() const { return _requestedItemId; }
	uint16_t contributedItemId() const { return _contributedItemId; }

	// Whether or not the negotiation finished
	bool negotiationFinished() const;

	// Whether or not there was a negotiation agreement
	bool negotiationAgreement() const;

	void SetParent(MCP * paren);
	bool HasParent() const;

	void SetDeepnees(uint16_t set);
	uint16_t GetDeepnees() const;

	bool IsChild();

	void SetNegotiationFinishedState();

public:
	// Negotiation control -------------------------

	void GetMCCsWithItem();

	void InitMCCsNegotiationList(std::vector<AgentLocation> agents);
	void StartCurrentMCCNegotiation();
	void SetNextMCC();

	void HandleMCCNegotiationResponse(const TCPSocketPtr& socket, bool response, uint16_t ucc_id);

	void ChildUCPSolutionFound();
	void ChildUCPNegotiationNotFound();

	void ChildMCPSolutionFound();
	void ChildMCPSolutionNotFound();

	// ---------------------------------------------

private:
	// Communication control -----------------------

	bool GetMCCsWithItem_SendToYellowPages(int itemId);
	bool StartNegotation_SendToMCC(const AgentLocation& mcc);
	bool FinishNegotiation_SendToMCC(const AgentLocation& mcc, bool succes);
	// ---------------------------------------------

	// Spawning control ----------------------------

	void createChildUCP();
	void destroyChildUCP();
	bool ChildUCPExists();

public:
	void createChildMCP(uint16_t requestedItemId);
	void destroyChildMCP();
	bool ChildMCPExists();

	// ---------------------------------------------

private:
	uint16_t _requestedItemId;
	uint16_t _contributedItemId;

	MCP* parent_mcp = nullptr;
	MCP* child_mcp = nullptr;

	UCP* child_ucp = nullptr; /**< Child UCP. */

	int _mccRegisterIndex; /**< Iterator through _mccRegisters. */
	std::vector<AgentLocation> _mccRegisters; /**< MCCs returned by the YP. */

	bool negotiation_agreement = false;

	bool is_child = false;

	uint16_t deepness = 0;
};

