#pragma once
#include "Globals.h"
#include "AgentLocation.h"

/**
 * Enumerated type for packets.
 * There must be a value for each kind of packet
 * containing extra data besides the Header.
 */
enum class PacketType
{
	// MCC <-> YP
	RegisterMCC,
	RegisterMCCAck,
	UnregisterMCC,

	// MCP <-> YP
	QueryMCCsForItem,
	ReturnMCCsForItem,
	
	// MCP <-> MCC
	MCPToMCCNegotiationRequest,
	MCCToMCPNegotiationResponse,
	MCPToMCCNegotiationFinish,
	MCCToMCPConnectionFinished,
	
	// UCP <-> UCC
	UCPToUCCNegotiationRequest,
	UCCToUCPNegotiationResponse,
	
	Last
};

/**
 * Standard information used by almost all messages in the system.
 * Agents will be communicating among each other, so in many cases,
 * besides the packet type, a header containing the source and the
 * destination agents involved is needed.
 */
class PacketHeader 
{
public:
	PacketType packetType; // Which type is this packet
	uint16_t srcAgentId;   // Which agent sent this packet?
	uint16_t dstAgentId;   // Which agent is expected to receive the packet?
	PacketHeader() :
		packetType(PacketType::Last),
		srcAgentId(NULL_AGENT_ID),
		dstAgentId(NULL_AGENT_ID)
	{ }

	void Deserialize(InputMemoryStream &stream) 
	{
		stream.Read(packetType);
		stream.Read(srcAgentId);
		stream.Read(dstAgentId);
	}

	void Serialize(OutputMemoryStream &stream) 
	{
		stream.Write(packetType);
		stream.Write(srcAgentId);
		stream.Write(dstAgentId);
	}
};

/**
 * To register a MCC we need to know which resource/item is
 * being provided by the MCC agent.
 */
class PacketRegisterMCC 
{
public:
	uint16_t itemId; // Which item has to be registered?

	void Deserialize(InputMemoryStream &stream) 
	{
		stream.Read(itemId);
	}

	void Serialize(OutputMemoryStream &stream) 
	{
		stream.Write(itemId);
	}
};

/**
* The information is the same required for PacketRegisterMCC so...
*/
using PacketUnregisterMCC = PacketRegisterMCC;

/**
* The information is the same required for PacketRegisterMCC so...
*/
using PacketQueryMCCsForItem = PacketRegisterMCC;

/**
 * This packet is the response for PacketQueryMCCsForItem and
 * is sent by an MCP (MultiCastPetitioner) agent.
 * It contains a list of the addresses of MCC agents contributing
 * with the item specified by the PacketQueryMCCsForItem.
 */
class PacketReturnMCCsForItem 
{
public:
	std::vector<AgentLocation> mccAddresses;

	void Deserialize(InputMemoryStream &stream)
	{
		uint16_t count;
		stream.Read(count);

		mccAddresses.resize(count);
		for (auto &mccAddress : mccAddresses) 
		{
			mccAddress.Deserialize(stream);
		}
	}

	void Serialize(OutputMemoryStream &stream)
	{
		auto count = static_cast<uint16_t>(mccAddresses.size());

		stream.Write(count);

		for (auto &mccAddress : mccAddresses) 
		{
			mccAddress.Serialize(stream);
		}
	}
};

class PacketMCPToMCCNegotiationRequest
{
public:
	uint16_t mcp_request = 0;
	uint16_t mcp_offer = 0;

	void Deserialize(InputMemoryStream &stream)
	{
		stream.Read(mcp_request);
		stream.Read(mcp_offer);
	}

	void Serialize(OutputMemoryStream &stream)
	{
		stream.Write(mcp_request);
		stream.Write(mcp_offer);
	}
};

class PacketMCCToMCPNegotiationResponse
{
public:
	bool response = false;

	uint16_t UCCId = 0; // We only use it in case the response it's true

	void Deserialize(InputMemoryStream &stream)
	{
		stream.Read(response);
		stream.Read(UCCId);
	}

	void Serialize(OutputMemoryStream &stream)
	{
		stream.Write(response);
		stream.Write(UCCId);
	}
};

class PacketUCPToUCCNegotiationRequest
{
public:
	uint16_t ucp_request = 0;
	uint16_t ucp_offer = 0;

	void Deserialize(InputMemoryStream &stream)
	{
		stream.Read(ucp_request);
		stream.Read(ucp_offer);
	}

	void Serialize(OutputMemoryStream &stream)
	{
		stream.Write(ucp_request);
		stream.Write(ucp_offer);
	}
};

class PacketUCCToUCPNegotiationResponse
{
public:
	bool response = false;
	bool solution_found = false;

	uint16_t UCCconstraint = 0; // We only use it in case the response it's true and solution found false

	void Deserialize(InputMemoryStream &stream)
	{
		stream.Read(response);
		stream.Read(solution_found);
		stream.Read(UCCconstraint);
	}

	void Serialize(OutputMemoryStream &stream)
	{
		stream.Write(response);
		stream.Write(solution_found);
		stream.Write(UCCconstraint);
	}
};
