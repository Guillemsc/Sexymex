#include "ModuleNodeCluster.h"
#include "ModuleNetworkManager.h"
#include "ModuleAgentContainer.h"
#include "Application.h"
#include "Log.h"
#include "Packets.h"
#include "imgui/imgui.h"
#include <sstream>

enum State 
{
	STOPPED,
	STARTING,
	RUNNING,
	STOPPING
};

bool ModuleNodeCluster::init()
{
	state = STOPPED;

	return true;
}

bool ModuleNodeCluster::start()
{
	state = STARTING;

	return true;
}

bool ModuleNodeCluster::update()
{
	bool ret = true;

	switch (state)
	{
	case STARTING:
		if (startSystem()) 
		{
			state = RUNNING;
		} 
		else 
		{
			state = STOPPED;
			ret = false;
		}
		break;
	case RUNNING:
		runSystem();

		break;
	case STOPPING:
		stopSystem();
		state = STOPPED;

		break;
	}

	return ret;
}

bool ModuleNodeCluster::updateGUI()
{
	ImGui::Begin("Node cluster");

	if (state == RUNNING)
	{
		// Number of sockets
		App->networkManager->drawInfoGUI();

		// Number of agents
		App->agentContainer->drawInfoGUI();

		ImGui::CollapsingHeader("ModuleNodeCluster", ImGuiTreeNodeFlags_DefaultOpen);

		int itemsCount = 0;
		for (auto node : _nodes) {
			itemsCount += (int)node->itemList().numItems();
		}
		ImGui::TextWrapped("# items in the cluster: %d", itemsCount);

		int missingItemsCount = 0;
		for (auto node : _nodes) {
			missingItemsCount += (int)node->itemList().numMissingItems();
		}
		ImGui::TextWrapped("# missing items in the cluster: %d", missingItemsCount);

		ImGui::Separator();

		if (ImGui::Button("Create random MCCs"))
		{
			for (NodePtr node : _nodes)
			{
				for (ItemId contributedItem = 0; contributedItem < MAX_ITEMS; ++contributedItem)
				{
					if (node->itemList().numItemsWithId(contributedItem) > 1)
					{
						unsigned int numItemsToContribute = node->itemList().numItemsWithId(contributedItem) -  1;

						for (ItemId constraintItem = 0; constraintItem < MAX_ITEMS; ++constraintItem)
						{
							if (node->itemList().numItemsWithId(constraintItem) == 0)
							{
								for (unsigned int i = 0; i < numItemsToContribute; ++i)
								{
									spawnMCC(node->id(), contributedItem, constraintItem);
								}
							}
						}
					}
				}
			}
		}

		if (ImGui::Button("Clear all agents"))
		{
			for (AgentPtr agent : App->agentContainer->allAgents())
			{
				agent->stop();
			}
		}

		ImGui::Separator();

		int nodeId = 0;
		for (auto &node : _nodes)
		{
			ImGui::PushID(nodeId);

			ImGuiTreeNodeFlags flags = 0;
			std::string nodeLabel = StringUtils::Sprintf("Node %d", nodeId);

			if (ImGui::CollapsingHeader(nodeLabel.c_str(), flags))
			{
				if (ImGui::TreeNodeEx("Items", flags))
				{
					auto &itemList = node->itemList();

					for (int itemId = 0; itemId < MAX_ITEMS; ++itemId)
					{
						unsigned int numItems = itemList.numItemsWithId(itemId);
						if (numItems == 1)
						{
							ImGui::Text("Item %d", itemId);
						}
						else if (numItems > 1)
						{
							ImGui::Text("Item %d (x%d)", itemId, numItems);
						}
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("MCCs", flags))
				{
					for (auto agent : App->agentContainer->allAgents()) {
						MCC * mcc = agent->asMCC();
						if (mcc != nullptr && mcc->node()->id() == nodeId)
						{
							ImGui::Text("MCC %d", mcc->id());
							ImGui::Text(" - Contributed Item ID: %d", mcc->contributedItemId());
							ImGui::Text(" - Constraint Item ID: %d", mcc->constraintItemId());
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("MCPs", flags))
				{
					for (auto agent : App->agentContainer->allAgents()) {
						MCP * mcp = agent->asMCP();
						if (mcp != nullptr && mcp->node()->id() == nodeId)
						{
							ImGui::Text("MCP %d", mcp->id());
							ImGui::Text(" - Requested Item ID: %d", mcp->requestedItemId());
							ImGui::Text(" - Contributed Item ID: %d", mcp->contributedItemId());
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
			nodeId++;
		}
	}

	ImGui::End();

	if (state == RUNNING)
	{
		// NODES / ITEMS MATRIX /////////////////////////////////////////////////////////

		ImGui::Begin("Nodes/Items Matrix");

		static ItemId selectedItem = 0;
		static unsigned int selectedNode = 0;
		static int comboItem = 0;

		ImGui::Text("Item ID ");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 0.5f));
		for (ItemId itemId = 0U; itemId < MAX_ITEMS; ++itemId)
		{
			ImGui::SameLine();
			std::ostringstream oss;
			oss << itemId;
			ImGui::Button(oss.str().c_str(), ImVec2(20, 20));
			if (itemId < MAX_ITEMS - 1) ImGui::SameLine();
		}
		ImGui::PopStyleColor(3);

		ImGui::Separator();

		for (auto nodeIndex = 0U; nodeIndex < _nodes.size(); ++nodeIndex)
		{
			ImGui::Text("Node %02u ", nodeIndex);
			ImGui::SameLine();

			for (ItemId itemId = 0U; itemId < MAX_ITEMS; ++itemId)
			{
				unsigned int numItems = _nodes[nodeIndex]->itemList().numItemsWithId(itemId);

				if (numItems == 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.2f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.5f*numItems));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(.0f, 1.0f, 0.0f, 0.3f*numItems));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.0f, 1.0f, 0.0f, 0.2f*numItems));
				}

				const int buttonId = nodeIndex * MAX_ITEMS + itemId;
				std::ostringstream oss;
				oss << numItems;
				oss << "##" << buttonId;

				if (ImGui::Button(oss.str().c_str(), ImVec2(20, 20)))
				{
					if (numItems == 0)
					{
						selectedNode = nodeIndex;
						selectedItem = itemId;
						comboItem = 0;
						ImGui::OpenPopup("ItemOps");
					}
				}

				ImGui::PopStyleColor(3);

				if (itemId < MAX_ITEMS - 1) ImGui::SameLine();
			}
		}

		// Context menu to spawn agents
		if (ImGui::BeginPopup("ItemOps"))
		{
			int numberOfItems = _nodes[selectedNode]->itemList().numItemsWithId(selectedItem);

			// If it is a missing item...
			if (numberOfItems == 0)
			{
				int requestedItem = selectedItem;

				// Check if we have spare items
				std::vector<std::string> comboStrings;
				std::vector<int> itemIds;
				for (ItemId itemId = 0; itemId < MAX_ITEMS; ++itemId) {
					if (_nodes[selectedNode]->itemList().numItemsWithId(itemId) > 1)
					{
						std::ostringstream oss;
						oss << itemId;
						comboStrings.push_back(oss.str());
						itemIds.push_back(itemId);
					}
				}

				std::vector<const char *> comboCStrings;
				for (auto &s : comboStrings) { comboCStrings.push_back(s.c_str()); }

				if (itemIds.size() > 0)
				{
					ImGui::Text("Create MultiCastPetitioner?");
					ImGui::Separator();
					ImGui::Text("Node %d", selectedNode);
					ImGui::Text(" - Petition: %d", requestedItem);

					ImGui::Combo("Contribution", &comboItem, (const char **)&comboCStrings[0], (int)comboCStrings.size());
					if (ImGui::Button("Spawn MCP")) {
						int contributedItem = itemIds[comboItem];
						spawnMCP(selectedNode, requestedItem, contributedItem);
						ImGui::CloseCurrentPopup();
					}
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
					ImGui::Text("No spare items available to create an MCP.");
					ImGui::PopStyleColor(1);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	return true;
}

bool ModuleNodeCluster::stop()
{
	state = STOPPING;

	return true;
}

bool ModuleNodeCluster::cleanUp()
{
	return true;
}

void ModuleNodeCluster::OnAccepted(TCPSocketPtr socket)
{
	// Nothing to do
}

void ModuleNodeCluster::OnPacketReceived(TCPSocketPtr socket, InputMemoryStream & stream)
{
	//iLog << "OnPacketReceived";

	PacketHeader packetHead;
	packetHead.Deserialize(stream);

	// Get the agent
	auto agentPtr = App->agentContainer->getAgent(packetHead.dstAgentId);

	if (agentPtr != nullptr)
	{
		agentPtr->OnPacketReceived(socket, packetHead, stream);
	}
	else
	{
		eLog << "Couldn't find agent: " << packetHead.dstAgentId;
	}
}

void ModuleNodeCluster::OnDisconnected(TCPSocketPtr socket)
{
	// Nothing to do
}

bool ModuleNodeCluster::startSystem()
{
	iLog << "--------------------------------------------";
	iLog << "           SiSiMEX: Node Cluster            ";
	iLog << "--------------------------------------------";
	iLog << "";

	// Create listen socket
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	if (listenSocket == nullptr) {
		eLog << "SocketUtil::CreateTCPSocket() failed";
		return false;
	}
	iLog << " - Server Listen socket created";

	// Bind
	const int port = LISTEN_PORT_AGENTS;
	SocketAddress bindAddress(port); // localhost:LISTEN_PORT_AGENTS
	listenSocket->SetReuseAddress(true);
	int res = listenSocket->Bind(bindAddress);
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket Bind to interface 127.0.0.1:" << LISTEN_PORT_AGENTS;

	// Listen mode
	res = listenSocket->Listen();
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket entered in Listen state...";

	// Add the socket to the manager
	App->networkManager->SetDelegate(this);
	App->networkManager->AddSocket(listenSocket);

#ifdef RANDOM_INITIALIZATION

	// Initialize nodes
	for (int i = 0; i < MAX_NODES; ++i)
	{
		// Create and intialize nodes
		NodePtr node = std::make_shared<Node>(i);
		node->itemList().initializeComplete();
		_nodes.push_back(node);
	}

	// Randomize
	for (int j = 0; j < MAX_ITEMS; ++j)
	{
		for (int i = 0; i < MAX_NODES; ++i)
		{
			ItemId itemId = rand() % MAX_ITEMS;
			while (_nodes[i]->itemList().numItemsWithId(itemId) == 0) 
			{
				itemId = rand() % MAX_ITEMS;
			}
			_nodes[i]->itemList().removeItem(itemId);
			_nodes[(i + 1) % MAX_NODES]->itemList().addItem(itemId);
		}
	}

#else
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes.push_back(std::make_shared<Node>((int)_nodes.size()));
	_nodes[0]->itemList().addItem(ItemId(0));
	_nodes[0]->itemList().addItem(ItemId(0));
	_nodes[1]->itemList().addItem(ItemId(1));
	_nodes[1]->itemList().addItem(ItemId(1));
	_nodes[2]->itemList().addItem(ItemId(2));
	_nodes[2]->itemList().addItem(ItemId(2));
	_nodes[3]->itemList().addItem(ItemId(3));
	_nodes[3]->itemList().addItem(ItemId(3));

	// Defines to clarify the next lines
#	define NODE(x) x
#	define CONTRIBUTION(x) x
#	define CONSTRAINT(x) x

	spawnMCC(NODE(1), CONTRIBUTION(1), CONSTRAINT(2)); // Node 1 offers 1 but wants 2
	spawnMCC(NODE(2), CONTRIBUTION(2), CONSTRAINT(3)); // Node 2 offers 2 but wants 3
	spawnMCC(NODE(3), CONTRIBUTION(3), CONSTRAINT(0)); // Node 3 offers 3 but wants 0

	//spawnMCC(0, 0); // Node 0 offers 0
	//spawnMCP(0, 1); // Node 0 wants  1

#endif

	return true;
}

void ModuleNodeCluster::runSystem()
{
	// Check the results of agents
	for (AgentPtr agent : App->agentContainer->allAgents())
	{
		if (!agent->isValid()) { continue; }

		// Update ItemList with finalized MCCs
		MCC *mcc = agent->asMCC();
		if (mcc != nullptr && mcc->negotiationFinished())
		{
			Node *node = mcc->node();
			node->itemList().removeItem(mcc->contributedItemId());
			node->itemList().addItem(mcc->constraintItemId());
			mcc->stop();
			iLog << "MCC exchange at Node " << node->id() << ":"
				<< " -" << mcc->contributedItemId()
				<< " +" << mcc->constraintItemId();
		}

		//// Update ItemList with MCPs that found a solution
		//MCP *mcp = agent->asMCP();
		//if (mcp != nullptr && mcp->negotiationFinished() && mcp->searchDepth() == 0)
		//{
		//	Node *node = mcp->node();

		//	if (mcp->negotiationAgreement())
		//	{
		//		node->itemList().addItem(mcp->requestedItemId());
		//		node->itemList().removeItem(mcp->contributedItemId());
		//		iLog << "MCP exchange at Node " << node->id() << ":"
		//			<< " -" << mcp->contributedItemId()
		//			<< " +" << mcp->requestedItemId();
		//	}
		//	else
		//	{
		//		wLog << "MCP exchange at Node " << node->id() << " not found:"
		//			<< " -" << mcp->contributedItemId()
		//			<< " +" << mcp->requestedItemId();
		//	}
		//	mcp->stop();
		//}
	}

	// WARNING:
	// The list of items of each node can change at any moment if a multilateral exchange took place
	// The following lines looks for agents which, after an update of items, make sense no more, and stops them
	for (AgentPtr agent : App->agentContainer->allAgents())
	{
		if (!agent->isValid()) 
			continue;

		Node *node = agent->node();
		MCC *mcc = agent->asMCC();
		if (mcc != nullptr && mcc->isIdling())
		{
			int numContributedItems = node->itemList().numItemsWithId(mcc->contributedItemId());
			int numRequestedItems = node->itemList().numItemsWithId(mcc->constraintItemId());

			if (numContributedItems < 2 || numRequestedItems > 0) 
			{ // if the contributed is not repeated at least once... or we already got the constraint
				mcc->stop();
			}
		}
	}
}

void ModuleNodeCluster::stopSystem()
{
}

void ModuleNodeCluster::spawnMCP(int nodeId, int requestedItemId, int contributedItemId)
{
	dLog << "Spawn MCP - node " << nodeId << " - req. " << requestedItemId << " - contrib. " << contributedItemId;

	if (nodeId >= 0 && nodeId < (int)_nodes.size()) 
	{
		NodePtr node = _nodes[nodeId];
		App->agentContainer->createMCP(node.get(), requestedItemId, contributedItemId, 0);
	}
	else 
	{
		wLog << "Could not find node with ID " << nodeId;
	}
}

void ModuleNodeCluster::spawnMCC(int nodeId, int contributedItemId, int constraintItemId)
{
	dLog << "Spawn MCC - node " << nodeId << " contrib. " << contributedItemId << " - constr. " << constraintItemId;

	if (nodeId >= 0 && nodeId < (int)_nodes.size()) 
	{
		NodePtr node = _nodes[nodeId];
		App->agentContainer->createMCC(node.get(), contributedItemId, constraintItemId);
	}
	else 
	{
		wLog << "Could not find node with ID " << nodeId;
	}
}

UCCPtr ModuleNodeCluster::spawnUCC(MCC* mcc)
{
	if (mcc != nullptr)
	{
		dLog << "Spawn UCC - node " << mcc->node()->id() << " contrib. " << mcc->contributedItemId() << " - constr. " << mcc->constraintItemId();

		if (mcc->node()->id() > 0)
		{
			return App->agentContainer->createUCC(mcc->node(), mcc->contributedItemId(), mcc->constraintItemId());
		}
		else
		{
			wLog << "Could not create UCC";
		}
	}

	return UCCPtr();
}

UCPPtr ModuleNodeCluster::spawnUCP(MCP* mcp)
{
	if (mcp != nullptr)
	{
		dLog << "Spawn UCP - node " << mcp->node()->id() << " contrib. " << mcp->contributedItemId() << " - constr. " << mcp->requestedItemId();

		if (mcp->node()->id() > 0)
		{
			return App->agentContainer->createUCP(mcp->node(), mcp->contributedItemId(), mcp->requestedItemId());
		}
	}

	return UCPPtr();
}
