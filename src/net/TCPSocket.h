#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

class TCPSocket;

class OutputMemoryStream;
class InputMemoryStream;

typedef std::shared_ptr<TCPSocket> TCPSocketPtr;

class TCPSocket
{
public:

	~TCPSocket();
	
	int Bind(const SocketAddress &inToAddress);
	int Listen(int inBackLog = 32);
	TCPSocketPtr Accept(SocketAddress &inFromAddress);
	int Connect(const SocketAddress &inAddress);
	int Send(const void *inData, int inLen);
	int Receive(void *inBuffer, int inLen);
	void Disconnect();

	int SetNonBlockingMode(bool inShouldBeNonBlocking);
	int SetReuseAddress(bool inShouldReuseAddress);

	bool IsListening() const { return mFlags & FlagListening; }
	bool ToDisconnect() const { return mFlags & FlagToDisconnect; }
	bool IsDisconnected() const { return mFlags & FlagDisconnected; }
	const SocketAddress &RemoteAddress() { return mRemoteAddress; }

	// Use these methods instead of Send / Receive in conjunction with
	// non-blocking methods (e.g. select)
	void SendPacket(const void *data, size_t size);
	bool ReceivePacket(void *data, size_t size);
	void SendPacket(OutputMemoryStream& outStream);
	bool ReceivePacket(InputMemoryStream& inStream);

	// Use these methods instead of Send / Receive in conjunction with
	// non-blocking methods (e.g. select)
	bool HasOutgoingData() const;
	void HandleOutgoingData();
	void HandleIncomingData();

private:

	friend class SocketUtil;

	// Only the network manager can call this explicitly
	friend class TCPNetworkManager;
	void CloseSocket();

	TCPSocket(SOCKET inSocket) :
		mSocket(inSocket),
		mFlags(0),
		mOutgoingDataHead(0), mOutgoingDataSendHead(0),
		mIncomingDataHead(0), mIncomingDataRecvHead(0)
	{ }

	enum Flag {
		FlagListening    = 1,
		FlagDisconnected = 2,
		FlagToDisconnect = 4
	};

	SOCKET mSocket;
	int mFlags;
	SocketAddress mRemoteAddress;

	// Data to be sent
	size_t mOutgoingDataHead; // Accumulated
	size_t mOutgoingDataSendHead; // Already sent
	std::vector<char> mOutgoingData;

	// Received data
	size_t mIncomingDataHead; // To process
	size_t mIncomingDataRecvHead; // Already read
	std::vector<char> mIncomingData;
};

#endif // TCP_SOCKET_H
