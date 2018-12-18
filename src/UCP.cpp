#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


// TODO: Make an enum with the states


UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId) :
	Agent(node)
{
	this->negotiating_ucc_id = negotiating_ucc_id;
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
		// TODO: Handle states

	default:;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::StartUCCNegotiation(const TCPSocketPtr & socket, uint16_t negotiating_ucc_id)
{
	this->negotiating_ucc_id = negotiating_ucc_id;
}
