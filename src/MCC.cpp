#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"
#include "ModuleNodeCluster.h"

enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	
	// TODO: Other states
	ST_NEGOTIATING,

	ST_FINISHED,
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
		case ST_INIT:
		{
			if (_contributedItemId == 1 && node()->id() == 3)
			{
				int i = id();

				int f = 0;
			}

			if (Register_SendToYellowPages())
			{
				setState(ST_REGISTERING);
			}
			else
			{
				setState(ST_FINISHED);
			}
			break;
		}
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	destroyChildUCC();

	Unregister_SendToYellowPages();
	setState(ST_FINISHED);

	destroy();

	wLog << "MCC Destroyed";
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
	{
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		}

		socket->Disconnect();

		break;
	}

	case PacketType::MCPToMCCNegotiationRequest:
	{		
		PacketMCPToMCCNegotiationRequest packetData;
		packetData.Deserialize(stream);

		HandleMCPNegotiationRequest(socket, packetHeader.srcAgentId, packetData.mcp_offer, packetData.mcp_request);
		
		break;
	}

	case PacketType::MCPToMCCNegotiationFinish:
	{
		PacketMCPToMCCNegotiationFinish packetData;
		packetData.Deserialize(stream);

		if (packetData.succes)
		{
			if (state() == ST_NEGOTIATING)
			{
				App->modNodeCluster->AddNodeOperation(node(), NodeOperationType::REMOVE, contributedItemId());
				App->modNodeCluster->AddNodeOperation(node(), NodeOperationType::ADD, constraintItemId());

				iLog << "MCC exchange at Node " << node()->id() << ":"
					<< " -" << contributedItemId()
					<< " +" << constraintItemId();

				setState(State::ST_FINISHED);

				ConnectionFinished_SendToMCP(socket, packetHeader.srcAgentId);
			}
		}
		else
		{
			setState(State::ST_IDLE);

			socket->Disconnect();
		}

		break;
	}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return negotiationFinished();
}

void MCC::SetParent(MCC * parent)
{
	this->parent = parent;
}

void MCC::HandleMCPNegotiationRequest(const TCPSocketPtr& socket, uint16_t mcp_id, uint16_t mcp_offer, uint16_t mcp_request)
{
	bool negotiate = false;
	uint16_t ucc_id = 0;

	if (state() == State::ST_IDLE)
	{
		if (_contributedItemId == 1 && node()->id() == 3)
		{
			int i = id();

			int f = 0;
		}

		if (mcp_request == _contributedItemId)
			negotiate = true;

		if (negotiate)
		{
			setState(State::ST_NEGOTIATING);

			createChildUCC();

			ucc_id = _ucc->id();
		}
	}
	else
	{
		int i = 0;
	}

	NegotiationResponse_SendToMCP(socket, mcp_id, negotiate, ucc_id);
}

bool MCC::Register_SendToYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;

	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::Unregister_SendToYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;

	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	sendPacketToYellowPages(stream);
}

void MCC::NegotiationResponse_SendToMCP(const TCPSocketPtr& socket, uint16_t mpc_id, bool response, uint16_t ucc_id)
{
	PacketHeader packetHead;
	packetHead.packetType = PacketType::MCCToMCPNegotiationResponse;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = mpc_id;

	PacketMCCToMCPNegotiationResponse packetData;
	packetData.response = response;
	packetData.UCCId = ucc_id;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);
	packetData.Serialize(stream);

	socket->SendPacket(stream);
}

void MCC::ConnectionFinished_SendToMCP(const TCPSocketPtr & socket, uint16_t mpc_id)
{
	PacketHeader packetHead;
	packetHead.packetType = PacketType::MCCToMCPConnectionFinished;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = mpc_id;

	OutputMemoryStream stream;
	packetHead.Serialize(stream);

	socket->SendPacket(stream);
}

void MCC::createChildUCC()
{
	destroyChildUCC();
		
	_ucc = App->modNodeCluster->spawnUCC(this);
}

void MCC::destroyChildUCC()
{
	if (UCCExists())
	{
		_ucc.reset();
	}
}

bool MCC::UCCExists()
{
	return _ucc.get();
}
