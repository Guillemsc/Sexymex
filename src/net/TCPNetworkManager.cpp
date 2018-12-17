#include "Net.h"
#include "TCPNetworkManager.h"

// Link with WinSockets library
#pragma comment(lib, "ws2_32.lib")


TCPNetworkManager::TCPNetworkManager() :
	mDelegate(nullptr)
{
}

TCPNetworkManager::~TCPNetworkManager()
{
}

void TCPNetworkManager::SetDelegate(TCPNetworkManagerDelegate * delegate)
{
	mDelegate = delegate;
}

void TCPNetworkManager::AddSocket(TCPSocketPtr socket)
{
	mSockets.push_back(socket);
}

void TCPNetworkManager::HandleSocketOperations(int timeoutMillis)
{
	// Preselect sockets for reading and writing
	std::vector<TCPSocketPtr> potentiallyReadableSockets;
	std::vector<TCPSocketPtr> potentiallyWritableSockets;

	for (auto socket : mSockets)
	{
		if (!socket->IsDisconnected())
		{
			potentiallyReadableSockets.push_back(socket);
			if (socket->HasOutgoingData())
			{
				potentiallyWritableSockets.push_back(socket);
			}
		}
	}

	// Select readable and writable sockets
	std::vector<TCPSocketPtr> readableSockets;
	std::vector<TCPSocketPtr> writableSockets;
	SocketUtil::Select(&potentiallyReadableSockets, &readableSockets, &potentiallyWritableSockets, &writableSockets, nullptr, nullptr, timeoutMillis);

	// Handle reading
	for (auto socket : readableSockets)
	{
		if (socket->IsListening())
		{
			SocketAddress fromAddress;
			TCPSocketPtr connectedSocket = socket->Accept(fromAddress);
			if (connectedSocket != nullptr)
			{
				mSockets.push_back(connectedSocket);
				mDelegate->OnAccepted(connectedSocket);
			}
		}
		else
		{
			socket->HandleIncomingData();

			if (!socket->IsDisconnected())
			{
				// 1) Crear in InputMemoryStream
				InputMemoryStream inputMemoryStream;

				while (socket->ReceivePacket(inputMemoryStream))
				{
					mDelegate->OnPacketReceived(socket, inputMemoryStream);
					inputMemoryStream.Clear();
				}
			}
		}
	}

	// Handle writing
	for (auto socket : writableSockets)
	{
		if (!socket->IsListening() && !socket->IsDisconnected()) // Maybe not needed... check
		{
			socket->HandleOutgoingData();
		}
	}

	// Handle socket disconnections
	std::vector<TCPSocketPtr> connectedSockets;
	for (auto socket : mSockets)
	{
		if (socket->ToDisconnect() && !socket->HasOutgoingData())
		{
			socket->CloseSocket();
		}

		if (socket->IsDisconnected())
		{
			mDelegate->OnDisconnected(socket);
		}
		else
		{
			connectedSockets.push_back(socket);
		}
	}

	mSockets.swap(connectedSockets);
}

void TCPNetworkManager::Finalize()
{
	// Finish sending pending outgoing data
	for (int i = 0; i < 100 ; ++i) 
	{
		bool pendingPackets = false;
		for (auto socket : mSockets) 
		{
			if (socket->HasOutgoingData()) 
				pendingPackets = true;
		}

		if (pendingPackets) 
			HandleSocketOperations();
		else 
			break;
	}

	// Disconnect all sockets
	for (auto socket : mSockets)
		socket->Disconnect();

	// Handle last disconnections
	HandleSocketOperations();

	// Clear sockets
	mSockets.clear();
}
