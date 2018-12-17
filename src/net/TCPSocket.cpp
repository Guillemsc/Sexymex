#include "Net.h"
#include <cstdint>

TCPSocket::~TCPSocket()
{
	CloseSocket();
}

int TCPSocket::Bind(const SocketAddress &inBindAddress)
{
	int err = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
	if (err != 0)
	{
		SocketUtil::ReportError("TCPSocket::Bind");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

int TCPSocket::Listen(int inBackLog)
{
	int err = listen(mSocket, inBackLog);
	if (err < 0)
	{
		SocketUtil::ReportError("TCPSocket::Listen");
		return -SocketUtil::GetLastError();
	}
	mFlags |= FlagListening;
	return NO_ERROR;
}

TCPSocketPtr TCPSocket::Accept(SocketAddress &inFromAddress)
{
	int length = inFromAddress.GetSize();
	SOCKET newSocket = accept(mSocket, &inFromAddress.mSockAddr, &length);

	if (newSocket != INVALID_SOCKET)
	{
		TCPSocketPtr socketPtr(new TCPSocket(newSocket));
		socketPtr->mRemoteAddress = inFromAddress;
		return socketPtr;
	}
	else
	{
		SocketUtil::ReportError("TCPSocket::Accept");
		return nullptr;
	}
}

int TCPSocket::Connect(const SocketAddress &inAddress)
{
	int err = connect(mSocket, &inAddress.mSockAddr, inAddress.GetSize());

	if (err < 0)
	{
		SocketUtil::ReportError("TCPSocket::Connect");
		return -SocketUtil::GetLastError();
	}

	mRemoteAddress = inAddress;

	return NO_ERROR;
}

int TCPSocket::Send(const void *inData, int inLen)
{
	int bytesSentCount = send(mSocket, static_cast<const char*>(inData), inLen, 0);
	if (bytesSentCount < 0)
	{
		auto lastError = SocketUtil::GetLastError();
		if (lastError != WSAEWOULDBLOCK && lastError != WSAEINPROGRESS) {
			SocketUtil::ReportError("TCPSocket::Send");
			mFlags |= FlagDisconnected;
		}
		return -lastError;
	}
	return bytesSentCount;
}

int TCPSocket::Receive(void *inBuffer, int inLen)
{
	int bytesReceivedCount = recv(mSocket, static_cast<char*>(inBuffer), inLen, 0);
	if (bytesReceivedCount < 0)
	{
		auto lastError = SocketUtil::GetLastError();
		if (lastError != WSAEWOULDBLOCK && lastError != WSAEINPROGRESS) {
			SocketUtil::ReportError("TCPSocket::Receive");
			mFlags |= FlagDisconnected;
		}
		return -lastError;
	}
	if (bytesReceivedCount == 0)
	{
		mFlags |= FlagDisconnected;
	}
	return bytesReceivedCount;
}

void TCPSocket::Disconnect()
{
	mFlags |= FlagToDisconnect;
}

int TCPSocket::SetNonBlockingMode(bool inShouldBeNonBlocking)
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
		SocketUtil::ReportError("TCPSocket::SetNonBlockingMode");
		return SocketUtil::GetLastError();
	}
	else
	{
		return NO_ERROR;
	}
}

int TCPSocket::SetReuseAddress(bool inShouldReuseAddress)
{
	int enable = (int)inShouldReuseAddress;
	int result = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (result == SOCKET_ERROR) {
		SocketUtil::ReportError("TCPSocket::SetReuseAddress");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

void TCPSocket::SendPacket(const void *data, size_t size)
{
	// Resize outgoing data buffer
	if (mOutgoingData.size() - mOutgoingDataHead < size + sizeof(uint32_t)) 
	{
		mOutgoingData.resize(mOutgoingData.size() + size + sizeof(uint32_t));
	}

	// Copy data size
	*(uint32_t*)(&mOutgoingData[mOutgoingDataHead]) = static_cast<uint32_t>(size);
	mOutgoingDataHead += sizeof(uint32_t);

	// Copy data
	memcpy((void*)&mOutgoingData[mOutgoingDataHead], data, size);
	mOutgoingDataHead += size;
}

bool TCPSocket::ReceivePacket(void *data, size_t size)
{
	bool read = false;
	uint32_t writeHead = 0;

	if (mIncomingDataRecvHead - mIncomingDataHead > sizeof(uint32_t))
	{
		const uint32_t packetSize = *(uint32_t*)&mIncomingData[mIncomingDataHead];
		const uint32_t availableSpaceInStream = (uint32_t)size - writeHead;
		if (mIncomingDataRecvHead - (mIncomingDataHead + sizeof(uint32_t)) >= packetSize &&
			packetSize <= availableSpaceInStream)
		{
			memcpy(
				(void*)((char*)data + writeHead),
				(const void*)&mIncomingData[mIncomingDataHead + sizeof(uint32_t)],
				packetSize);
			writeHead += packetSize;
			mIncomingDataHead += packetSize + sizeof(uint32_t);
			read = true;
		}
	}

	if (!read)
	{
		// Move heads to the beginning
		if (mIncomingDataHead >= mIncomingDataRecvHead) 
		{
			mIncomingDataHead = 0;
			mIncomingDataRecvHead = 0;
		}
		else 
		{
			size_t remainingBytes = mIncomingDataRecvHead - mIncomingDataHead;
			memmove((void*)&mIncomingData[0], (const void*)&mIncomingData[mIncomingDataHead], remainingBytes);
			mIncomingDataHead = 0;
			mIncomingDataRecvHead = remainingBytes;
		}
	}

	return read;
}

void TCPSocket::SendPacket(OutputMemoryStream& outStream)
{
	SendPacket(outStream.GetBufferPtr(), outStream.GetSize());
}

bool TCPSocket::ReceivePacket(InputMemoryStream& inStream)
{
	return ReceivePacket(inStream.GetBufferPtr(), inStream.GetCapacity());
}

bool TCPSocket::HasOutgoingData() const
{
	return mOutgoingDataHead != mOutgoingDataSendHead;
}

void TCPSocket::HandleOutgoingData()
{
	const int sentBytes = Send((const void*)&mOutgoingData[mOutgoingDataSendHead], (int)(mOutgoingDataHead - mOutgoingDataSendHead));
	if (sentBytes > 0)
	{
		mOutgoingDataSendHead += sentBytes;

		// Move heads to the beginning
		if (mOutgoingDataSendHead >= mOutgoingDataHead) {
			mOutgoingDataHead = 0;
			mOutgoingDataSendHead = 0;
		} else {
			size_t remainingBytes = mOutgoingDataHead - mOutgoingDataSendHead;
			memmove((void*)&mOutgoingData[0], (const void*)&mOutgoingData[mOutgoingDataSendHead], remainingBytes);
			mOutgoingDataHead = remainingBytes;
			mOutgoingDataSendHead = 0;
		}
	}
}

void TCPSocket::HandleIncomingData()
{
	// Resize incoming data buffer
	const size_t size = 1500 * 10;
	if (mIncomingData.size() - mIncomingDataRecvHead < size) {
		mIncomingData.resize(mIncomingData.size() + size);
	}

	const int recvBytes = Receive((void*)&mIncomingData[mIncomingDataRecvHead], size);
	if (recvBytes > 0) {
		mIncomingDataRecvHead += recvBytes;
	}
}

void TCPSocket::CloseSocket()
{
	if ((mFlags & FlagDisconnected) == 0)
	{
#ifdef _WIN32
		closesocket(mSocket);
#else
		close(mSocket);
#endif
		mFlags |= FlagDisconnected;
	}
}