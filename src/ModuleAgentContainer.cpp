#include "ModuleAgentContainer.h"
#include "MCC.h"
#include "MCP.h"
#include "UCC.h"
#include "UCP.h"
#include "imgui/imgui.h"


ModuleAgentContainer::ModuleAgentContainer()
{
}

ModuleAgentContainer::~ModuleAgentContainer()
{
}

MCCPtr ModuleAgentContainer::createMCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId)
{
	MCCPtr mcc(new MCC(node, contributedItemId, constraintItemId));
	addAgent(mcc);

	return mcc;
}

MCPPtr ModuleAgentContainer::createMCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, unsigned int searchDepth)
{
	MCPPtr mcp(new MCP(node, requestedItemId, contributedItemId, searchDepth));
	addAgent(mcp);

	return mcp;
}

UCCPtr ModuleAgentContainer::createUCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId)
{
	UCCPtr ucc(new UCC(node, contributedItemId, constraintItemId));
	addAgent(ucc);

	return ucc;
}

UCPPtr ModuleAgentContainer::createUCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth)
{
	UCPPtr ucp(new UCP(node, requestedItemId, contributedItemId, uccLocation, searchDepth));
	addAgent(ucp);

	return ucp;
}

void ModuleAgentContainer::addAgent(AgentPtr agent)
{
	_agentsToAdd.push_back(agent);
}

AgentPtr ModuleAgentContainer::getAgent(int agentId)
{
	// Agent search
	for (auto agent : _agents) 
	{
		if (agent->id() == agentId) 
		{
			return agent;
		}
	}

	for (auto agent : _agentsToAdd) 
	{
		if (agent->id() == agentId) 
		{
			return agent;
		}
	}

	return nullptr;
}

bool ModuleAgentContainer::empty() const
{
	return _agents.empty();
}

bool  ModuleAgentContainer::update()
{
	// Update all agents
	for (auto agent : _agents)
	{
		if (agent->isValid()) 
		{
			agent->update();
		}
	}

	return true;
}

bool ModuleAgentContainer::postUpdate()
{
	// Add pending agents to add
	for (auto agentToAdd : _agentsToAdd)
	{
		_agents.push_back(agentToAdd);
	}
	_agentsToAdd.clear();

	// Track alive agents
	std::vector<AgentPtr> agentsAlive;

	// Update all agents
	for (auto agent : _agents)
	{
		// Keep track of alive agents
		if (agent->isValid()) 
		{
			agentsAlive.push_back(agent);
		}
	}

	// Remove finished agents
	_agents.swap(agentsAlive);

	return true;
}

bool ModuleAgentContainer::stop()
{
	// Finalize all agents
	for (auto agent : _agents) 
		agent->stop();
	
	return true;
}

bool ModuleAgentContainer::cleanUp()
{
	_agents.clear();

	return true;
}

void ModuleAgentContainer::drawInfoGUI()
{
	int mccCount = 0;
	int mcpCount = 0;
	int uccCount = 0;
	int ucpCount = 0;

	for (auto agent : _agents) {
		if (agent->asMCC() != nullptr) { mccCount++; }
		if (agent->asMCP() != nullptr) { mcpCount++; }
		if (agent->asUCC() != nullptr) { uccCount++; }
		if (agent->asUCP() != nullptr) { ucpCount++; }
	}

	if (ImGui::CollapsingHeader("ModuleAgentContainer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped("# MCC agents: %d", mccCount);
		ImGui::TextWrapped("# MCP agents: %d", mcpCount);
		ImGui::TextWrapped("# UCC agents: %d", uccCount);
		ImGui::TextWrapped("# UCP agents: %d", ucpCount);
	}
}
