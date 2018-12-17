#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

class UDPSocket
{
public:

	~UDPSocket();

	int Bind(const SocketAddress &inToAddress);
	int SendTo(const void *inData, int inLen, const SocketAddress &inTo);
	int ReceiveFrom(void *inBuffer, int inLen, SocketAddress &outFrom);

	int SetNonBlockingMode(bool inShouldBeNonBlocking);
	int SetReuseAddress(bool inShouldReuseAddress);

private:

	friend class SocketUtil;
	UDPSocket(SOCKET inSocket) : mSocket(inSocket) { }
	SOCKET mSocket;
};

typedef std::shared_ptr<UDPSocket> UDPSocketPtr;

#endif // UDP_SOCKET_H
