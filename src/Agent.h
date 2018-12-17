#pragma once
#include "Globals.h"
#include "Log.h"
#include "Packets.h"
#include "Node.h"
#include <list>
#include <memory>

// Concrete agent declarations
class MCC;
class MCP;
class UCC;
class UCP;


class Agent
{
public:

	// Constructor and destructor /////////////////////////////////////

	Agent(Node *node);

	virtual ~Agent();


	// Lifetime methods ///////////////////////////////////////////////
	
	virtual void start() { }   // Called after creating the agent
	
	virtual void update() = 0; // Called once per frame

	virtual void stop() = 0;   // Called before destroying the agent
	
							   
	// Fast casts /////////////////////////////////////////////////////

	virtual MCC* asMCC() { return nullptr; }
	virtual MCP* asMCP() { return nullptr; }
	virtual UCC* asUCC() { return nullptr; }
	virtual UCP* asUCP() { return nullptr; }



	// State machine //////////////////////////////////////////////////

	int state() const { return _state; }

	void setState(int state) { _state = state; }


	// Networking methods /////////////////////////////////////////////

	// Packet send functions
	bool sendPacketToYellowPages(OutputMemoryStream &stream);
	bool sendPacketToAgent(const std::string &ip, uint16_t port, OutputMemoryStream &stream);

	// Function called from ModuleNodeCluster to forward packets received from the network
	virtual void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) = 0;


	// Schedule for destruction ///////////////////////////////////////

	// Is valid/alive? (if false, the agent will be removed from AgentManager)
	bool isValid() const { return _destroyFlag == false; }

protected:

	// Finish condition (if true, will be removed from AgentManager)
	// Should be called by the agent itself when it finished
	void destroy();

private:

	bool _destroyFlag; /*( Whether or not the agent finished and should be destroyed. )*/


public:

	/** It returns the parent node of the agent. */
	Node *node() const { return _node; }

	/** It returns the identifier of the agent (within the host) */
	uint16_t id() const { return _id; }

private:

	Node *_node; /**< Parent Node/player of the agent. */

	uint16_t _id; /**< Agent identifier. */

	int _state; /**< Current state of the agent. */

	std::vector<TCPSocketPtr> _sockets; /**< Sockets used from this agent. */
};

using AgentPtr = std::shared_ptr<Agent>;
