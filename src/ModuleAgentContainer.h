#pragma once

#include "Module.h"
#include <memory>
#include <vector>

class Node;
class Agent;
class AgentLocation;
class MCC;
class MCP;
class UCC;
class UCP;
using AgentPtr = std::shared_ptr<Agent>;
using MCCPtr = std::shared_ptr<MCC>;
using MCPPtr = std::shared_ptr<MCP>;
using UCCPtr = std::shared_ptr<UCC>;
using UCPPtr = std::shared_ptr<UCP>;

class ModuleAgentContainer : public Module
{
public:

	// Constructor and destructor
	ModuleAgentContainer();
	~ModuleAgentContainer();

	// Agent creation methods
	MCCPtr createMCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);
	MCPPtr createMCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, unsigned int searchDepth);
	UCCPtr createUCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId);
	UCPPtr createUCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth);

	// Getters
	AgentPtr getAgent(int agentId);
	std::vector<AgentPtr> &allAgents() { return _agents; }
	bool empty() const;

	// Update
	bool update() override;

	// Post update
	bool postUpdate() override;

	// Tell all agents to stop
	bool stop() override;

	// Remove all agents from memory
	bool cleanUp() override;


public:

	// GUI
	void drawInfoGUI();


private:

	// Setters
	void addAgent(AgentPtr agent);

	std::vector<AgentPtr> _agentsToAdd; /**< Agents to add. */
	std::vector<AgentPtr> _agents; /**< Array of agents. */
};