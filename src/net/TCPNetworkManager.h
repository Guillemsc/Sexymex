#pragma once
#include "Net.h"

class TCPNetworkManagerDelegate
{
public:

	virtual ~TCPNetworkManagerDelegate() { }

	virtual void OnAccepted(TCPSocketPtr socket) = 0;
	virtual void OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream) = 0;
	virtual void OnDisconnected(TCPSocketPtr socket) = 0;
};

class TCPNetworkManager
{
public:

	TCPNetworkManager();
	virtual ~TCPNetworkManager();

	void SetDelegate(TCPNetworkManagerDelegate *delegate);

	void AddSocket(TCPSocketPtr socket);

	void HandleSocketOperations(int timeoutMillis = 0);

	void Finalize();

protected:

	const std::vector<TCPSocketPtr> &allSockets() const { return mSockets; }

private:

	TCPNetworkManagerDelegate *mDelegate;
	std::vector<TCPSocketPtr> mSockets;
};

