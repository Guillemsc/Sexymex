#pragma once

#include "Module.h"
#include "net/Net.h"
#include "Node.h"
#include "MCC.h"
#include "MCP.h"

enum NodeOperationType
{
	ADD,
	REMOVE,
	END,
};

class NodeOperation
{
public:
	Node* node = nullptr;

	uint16_t value = 0;

	NodeOperationType type = NodeOperationType::END;
};

class ModuleNodeCluster : public Module, public TCPNetworkManagerDelegate
{
public:

	// Virtual methods from parent class Module

	bool init() override;

	bool start() override;

	bool update() override;

	bool updateGUI() override;

	bool cleanUp() override;

	bool stop() override;


	// TCPNetworkManagerDelegate virtual methods

	void OnAccepted(TCPSocketPtr socket) override;

	void OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream) override;

	void OnDisconnected(TCPSocketPtr socket) override;

	MCP* spawnMCP(int nodeId, int requestedItemId, int contributedItemId);
	void spawnMCC(int nodeId, int contributedItemId, int constraintItemId);
	UCCPtr spawnUCC(MCC* mcc);
	UCPPtr spawnUCP(MCP* mcp);

	void AddNodeOperation(Node* node, NodeOperationType type, uint16_t value);

private:

	bool startSystem();

	void runSystem();

	void stopSystem();

	void UpdateNodeOperations();

private:
	std::vector<NodePtr> _nodes; /**< Array of nodes spawn in this host. */
	std::vector<NodeOperation> node_operations;

	int state = 0; /**< State machine. */
};
