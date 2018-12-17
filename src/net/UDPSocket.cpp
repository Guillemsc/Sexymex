#include "Net.h"

UDPSocket::~UDPSocket()
{
#ifdef _WIN32
	closesocket(mSocket);
#else
	close(mSocket);
#endif
}

int UDPSocket::Bind(const SocketAddress &inBindAddress)
{
	int err = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
	if (err != 0)
	{
		SocketUtil::ReportError("UDPSocket::Bind");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

int UDPSocket::SendTo(const void *inData, int inLen, const SocketAddress &inTo)
{
	int byteSentCount = sendto(mSocket, static_cast<const char*>(inData), inLen, 0, &inTo.mSockAddr, inTo.GetSize());
	if (byteSentCount >= 0)
	{
		return byteSentCount;
	}
	else
	{
		auto lastError = SocketUtil::GetLastError();
		if (lastError != EWOULDBLOCK) {
			SocketUtil::ReportError("UDPSocket::SendTo");
		}
		return -lastError;
	}
}

int UDPSocket::ReceiveFrom(void *inBuffer, int inLen, SocketAddress &outFrom)
{
	int fromLength = outFrom.GetSize();
	int readByteCount = recvfrom(mSocket, static_cast<char*>(inBuffer), inLen, 0, &outFrom.mSockAddr, &fromLength);
	if (readByteCount >= 0)
	{
		return readByteCount;
	}
	else
	{
		auto lastError = SocketUtil::GetLastError();
		if (lastError != EWOULDBLOCK) {
			SocketUtil::ReportError("UDPSocket::ReceiveFrom");
		}
		return -lastError;
	}
}


int UDPSocket::SetNonBlockingMode(bool inShouldBeNonBlocking)
{
#if _WIN32
	u_long arg = inShouldBeNonBlocking ? 1 : 0;
	int result = ioctlsocket(mSocket, FIONBIO, &arg);
#else
	int flags = fcntl(mSocket, F_GETFL, 0);
	flags = inShouldBeNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	int result = fcntl(mSocket, F_SETFL, flags);
#endif

	if (result == SOCKET_ERROR)
	{
		SocketUtil::ReportError("UDPSocket::SetNonBlockingMode");
		return SocketUtil::GetLastError();
	}
	else
	{
		return NO_ERROR;
	}
}

int UDPSocket::SetReuseAddress(bool inShouldReuseAddress)
{
	int enable = (int)inShouldReuseAddress;
	int result = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (result == SOCKET_ERROR) {
		SocketUtil::ReportError("UDPSocket::SetReuseAddress");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}
