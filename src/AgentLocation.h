#pragma once
#include "Globals.h"
#include "net/Net.h"

/**
* Basic location information about agents.
* It contains the minimum information to find an agent in
* the network (complete ip address + port + agent identifier)
*/
class AgentLocation
{
public:

	std::string hostIP; /**< IP address where the agent is. */
	uint16_t hostPort; /**< Listen port of this host. */
	uint16_t agentId; /**< Identifier of the MCC agent within the host. */

	void Deserialize(InputMemoryStream &stream) 
	{
		stream.Read(hostIP);
		stream.Read(hostPort);
		stream.Read(agentId);
	}

	void Serialize(OutputMemoryStream &stream) 
	{
		stream.Write(hostIP);
		stream.Write(hostPort);
		stream.Write(agentId);
	}
};
