#include "ModuleYellowPages.h"
#include "ModuleNetworkManager.h"
#include "Application.h"
#include "Packets.h"
#include "Log.h"
#include "imgui/imgui.h"

enum State 
{
	STOPPED,
	STARTING,
	RUNNING,
	STOPPING
};

bool ModuleYellowPages::init()
{
	state = STOPPED;

	return true;
}

bool ModuleYellowPages::start()
{
	state = STARTING;

	return true;
}

bool ModuleYellowPages::update()
{
	bool ret = true;

	switch (state)
	{
	case STARTING:
		if (startService()) 
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
		break;

	case STOPPING:
		stopService();
		state = STOPPED;
		break;
	}

	return ret;
}

bool ModuleYellowPages::updateGUI()
{
	ImGui::Begin("Yellow Pages");

	// Number of sockets
	App->networkManager->drawInfoGUI();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
	if (ImGui::CollapsingHeader("Registered MCCs", flags))
	{
		int imguiId = 0;
		for (auto &mcc : _mccByItem)
		{
			auto itemId = mcc.first;
			auto &agentLocations = mcc.second;

			if (ImGui::TreeNodeEx((void*)imguiId++, flags, "MCCs for item %d", (int)itemId))
			{
				for (auto &agentLocation : agentLocations)
				{
					ImGui::Text(" - %s:%d - agent:%d", agentLocation.hostIP.c_str(), agentLocation.hostPort, agentLocation.agentId);
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();

	return true;
}

bool ModuleYellowPages::stop()
{
	state = STOPPING;

	return true;
}

bool ModuleYellowPages::startService()
{
	iLog << "--------------------------------------------";
	iLog << "           SiSiMEX: Yellow Pages            ";
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
	const int port = LISTEN_PORT_YP;
	SocketAddress bindAddress(port); // localhost:LISTEN_PORT_YP
	listenSocket->SetReuseAddress(true);
	int res = listenSocket->Bind(bindAddress);
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket Bind to interface 127.0.0.1:" << LISTEN_PORT_YP;

	// Listen mode
	res = listenSocket->Listen();
	if (res != NO_ERROR) { return false; }
	iLog << " - Socket entered in Listen state...";

	// Add the socket to the manager
	App->networkManager->SetDelegate(this);
	App->networkManager->AddSocket(listenSocket);

	return true;
}

void ModuleYellowPages::stopService()
{
	// Nothing to do
}

void ModuleYellowPages::OnAccepted(TCPSocketPtr socket)
{
	// Nothing to do
}

void ModuleYellowPages::OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream)
{
	// Read packet header
	PacketHeader inPacketHead;
	inPacketHead.Deserialize(stream);

	switch (inPacketHead.packetType)
	{
		case PacketType::RegisterMCC:
		{
			// Read the packet
			PacketRegisterMCC inPacketData;
			inPacketData.Deserialize(stream);

			// Register the MCC into the yellow pages
			AgentLocation mcc;
			mcc.hostIP = socket->RemoteAddress().GetIPString();
			mcc.hostPort = LISTEN_PORT_AGENTS;
			mcc.agentId = inPacketHead.srcAgentId;
			_mccByItem[inPacketData.itemId].push_back(mcc);

			// Host address
			std::string hostAddress = socket->RemoteAddress().GetString();

			// Send RegisterMCCAck packet
			OutputMemoryStream outStream;
			PacketHeader outPacket;
			outPacket.packetType = PacketType::RegisterMCCAck;
			outPacket.dstAgentId = inPacketHead.srcAgentId;
			outPacket.Serialize(outStream);

			socket->SendPacket(outStream);

			break;
		}

		case PacketType::UnregisterMCC:
		{
			// Read the packet
			PacketUnregisterMCC inPacketData;
			inPacketData.Deserialize(stream);

			// Unregister the MCC from the yellow pages
			std::list<AgentLocation> &mccs(_mccByItem[inPacketData.itemId]);
			for (auto it = mccs.begin(); it != mccs.end();) 
			{
				if (it->agentId == inPacketHead.srcAgentId) 
				{
					auto oldIt = it++;
					mccs.erase(oldIt);
					break;
				}
				else 
					++it;
			}

			//// Send RegisterMCCAck packet
			//OutputMemoryStream outStream;
			//PacketHeader outPacket;
			//outPacket.packetType = PacketType::UnregisterMCCAck;
			//outPacket.dstAgentId = inPacketHead.srcAgentId;
			//outPacket.Write(outStream);
			//socket->SendPacket(outStream.GetBufferPtr(), outStream.GetSize());

			break;
		}

		case PacketType::QueryMCCsForItem:
		{
			// Read packet
			PacketQueryMCCsForItem inPacketData;
			inPacketData.Deserialize(stream);

			// Response packet
			PacketReturnMCCsForItem outPacketData;

			// Obtain the MCCAddresses
			auto itemId = inPacketData.itemId;
			auto &mccAddressList = _mccByItem[itemId];

			for (auto &mccAddress : mccAddressList) 
			{
				outPacketData.mccAddresses.push_back(mccAddress);
			}

			// Send response packet
			OutputMemoryStream outStream;

			PacketHeader outPacketHead;
			outPacketHead.packetType = PacketType::ReturnMCCsForItem;
			outPacketHead.dstAgentId = inPacketHead.srcAgentId;
			outPacketHead.Serialize(outStream);
			outPacketData.Serialize(outStream);

			socket->SendPacket(outStream);

			break;
		}

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
		break;
	}
}

void ModuleYellowPages::OnDisconnected(TCPSocketPtr socket)
{
	// Nothing to do
	//iLog << "Socket disconnected gracefully";
}
