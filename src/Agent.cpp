#include "Agent.h"
#include "Application.h"
#include "ModuleNetworkManager.h"

uint16_t g_IdCounter = 1;

Agent::Agent(Node *node) :
	_destroyFlag(false),
	_node(node),
	_id(g_IdCounter++)
{
}


Agent::~Agent()
{
}

bool Agent::sendPacketToYellowPages(OutputMemoryStream &stream)
{
	// Create socket
	TCPSocketPtr agentSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);

	if (agentSocket == nullptr) 
	{
		eLog << "SocketUtil::CreateTCPSocket() failed";
		return false;
	}

	// Connect to Yellow Pages
	char addressAndPort[128];
	sprintf_s(addressAndPort, "%s:%d", HOSTNAME_YP, LISTEN_PORT_YP);
	SocketAddress yellowPagesAddress(addressAndPort);

	int res = agentSocket->Connect(yellowPagesAddress);

	if (res != NO_ERROR) 
	{
		eLog << "TCPSocket::Connect() failed";
		return false;
	}

	// Add socket to the network manager
	App->networkManager->AddSocket(agentSocket);

	// Add socket to the list of sockets of this agent
	_sockets.push_back(agentSocket);

	// Append data
	agentSocket->SendPacket(stream);

	return true;
}

bool Agent::sendPacketToAgent(const std::string &ip, uint16_t port, OutputMemoryStream &stream)
{
	// Create socket
	TCPSocketPtr agentSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);

	if (agentSocket == nullptr) 
	{
		eLog << "SocketUtil::CreateTCPSocket() failed";
		return false;
	}

	// Connect to Yellow Pages
	char addressAndPort[128];
	sprintf_s(addressAndPort, "%s:%d", ip.c_str(), port);
	SocketAddress yellowPagesAddress(addressAndPort);
	int res = agentSocket->Connect(yellowPagesAddress);
	if (res != NO_ERROR) {
		eLog << "TCPSocket::Connect() failed";
		return false;
	}

	// Add socket to the network manager
	App->networkManager->AddSocket(agentSocket);

	// Add socket to the list of sockets of this agent
	_sockets.push_back(agentSocket);

	// Append data
	agentSocket->SendPacket(stream);

	return true;
}

void Agent::destroy()
{
	// Tell the AgentContainer to remove this Agent
	_destroyFlag = true;

	// Disconnect all sockets used by this agent
	for (auto socket : _sockets) {
		socket->Disconnect();
	}
	_sockets.clear();
}