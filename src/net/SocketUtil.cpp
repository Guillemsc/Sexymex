#include "Net.h"
#include "../Log.h" // Not nice log here...

bool SocketUtil::StaticInit()
{
#if _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		ReportError("Starting Up");
		return false;
	}
#endif
	return true;
}

void SocketUtil::CleanUp()
{
#if _WIN32
	WSACleanup();
#endif
}

void SocketUtil::ReportError(const char* inOperationDesc)
{
#if _WIN32
	wchar_t *lpMsgBuf;
	DWORD errorNum = WSAGetLastError();
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0, NULL);
	std::string msg = StringUtils::Sprintf("Error %s: %d- %S", inOperationDesc, errorNum, lpMsgBuf);
	wLog << msg.c_str();
	LocalFree(lpMsgBuf);
#else
	LOG("Error: %hs", inOperationDesc);
#endif
}

int SocketUtil::GetLastError()
{
#if _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

UDPSocketPtr SocketUtil::CreateUDPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);
	if (s != INVALID_SOCKET)
	{
		return UDPSocketPtr(new UDPSocket(s));
	}
	else
	{
		ReportError("SocketUtil::CreateUDPSocket");
		return nullptr;
	}
}

TCPSocketPtr SocketUtil::CreateTCPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_STREAM, IPPROTO_TCP);
	if (s != INVALID_SOCKET)
	{
		return TCPSocketPtr(new TCPSocket(s));
	}
	else
	{
		ReportError("SocketUtil::CreateTCPSocket");
		return nullptr;
	}
}

fd_set* SocketUtil::FillSetFromVector(fd_set& outSet, const std::vector< TCPSocketPtr >* inSockets, int& ioNaxNfds)
{
	if (inSockets)
	{
		FD_ZERO(&outSet);
		for (const TCPSocketPtr& socket : *inSockets)
		{
			FD_SET(socket->mSocket, &outSet);
#if !_WIN32
			ioNaxNfds = std::max(ioNaxNfds, socket->mSocket);
#endif
		}
		return &outSet;
	}
	else
	{
		return nullptr;
	}
}

fd_set* SocketUtil::FillSetFromVectorRange(fd_set& outSet, const std::vector< TCPSocketPtr >* inSockets, int& ioNaxNfds, int begin, int end)
{
	if (inSockets)
	{
		int i = begin;
		FD_ZERO(&outSet);
		for (int i = begin; i < end && i < (int)inSockets->size(); ++i)
		{
			const TCPSocketPtr &socket((*inSockets)[i]);

			FD_SET(socket->mSocket, &outSet);
#if !_WIN32
			ioNaxNfds = std::max(ioNaxNfds, socket->mSocket);
#endif
		}
		return &outSet;
	}
	else
	{
		return nullptr;
	}
}

void SocketUtil::FillVectorFromSet(std::vector< TCPSocketPtr >* outSockets, const std::vector< TCPSocketPtr >* inSockets, const fd_set& inSet)
{
	if (inSockets && outSockets)
	{
		outSockets->clear();
		for (const TCPSocketPtr& socket : *inSockets)
		{
			if (FD_ISSET(socket->mSocket, &inSet))
			{
				outSockets->push_back(socket);
			}
		}
	}
}

void SocketUtil::AddToVectorFromSetRange(std::vector<TCPSocketPtr>* outSockets, const std::vector<TCPSocketPtr>* inSockets, const fd_set & inSet, int begin, int end)
{
	if (inSockets && outSockets)
	{
		for (int i = begin; i < end && i < (int)inSockets->size(); ++i)
		{
			const TCPSocketPtr &socket((*inSockets)[i]);

			if (FD_ISSET(socket->mSocket, &inSet))
			{
				outSockets->push_back(socket);
			}
		}
	}
}

#if 1

int SocketUtil::Select(const std::vector< TCPSocketPtr >* inReadSet,
	std::vector< TCPSocketPtr >* outReadSet,
	const std::vector< TCPSocketPtr >* inWriteSet,
	std::vector< TCPSocketPtr >* outWriteSet,
	const std::vector< TCPSocketPtr >* inExceptSet,
	std::vector< TCPSocketPtr >* outExceptSet,
	int timeoutMillis)
{
	// Maximum number of sockets supported by select()
#	define MAX_SOCKETS 64

	int toRet = 0;

	//build up some sets from our vectors
	fd_set read, write, except;

	int begin = 0;
	bool finished = false;

	do
	{
		int nfds = 0;

		fd_set *readPtr = FillSetFromVectorRange(read, inReadSet, nfds, begin, begin + MAX_SOCKETS);
		fd_set *writePtr = FillSetFromVectorRange(write, inWriteSet, nfds, begin, begin + MAX_SOCKETS);
		fd_set *exceptPtr = FillSetFromVectorRange(except, inExceptSet, nfds, begin, begin + MAX_SOCKETS);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutMillis * 1000;

		toRet = select(nfds + 1, readPtr, writePtr, exceptPtr, &timeout);

		if (toRet > 0)
		{
			AddToVectorFromSetRange(outReadSet, inReadSet, read, begin, begin + 64);
			AddToVectorFromSetRange(outWriteSet, inWriteSet, write, begin, begin + 64);
			AddToVectorFromSetRange(outExceptSet, inExceptSet, except, begin, begin + 64);
		}

		begin += MAX_SOCKETS;

		finished =
			(inReadSet == nullptr || begin >= inReadSet->size()) &&
			(inWriteSet == nullptr || begin >= inWriteSet->size()) &&
			(inExceptSet == nullptr || begin >= inExceptSet->size());
	}
	while (!finished);

	return toRet;
}

#else

int SocketUtil::Select(const std::vector< TCPSocketPtr >* inReadSet,
	std::vector< TCPSocketPtr >* outReadSet,
	const std::vector< TCPSocketPtr >* inWriteSet,
	std::vector< TCPSocketPtr >* outWriteSet,
	const std::vector< TCPSocketPtr >* inExceptSet,
	std::vector< TCPSocketPtr >* outExceptSet,
	int timeoutMillis)
{
	//build up some sets from our vectors
	fd_set read, write, except;

	int nfds = 0;

	fd_set *readPtr = FillSetFromVector(read, inReadSet, nfds);
	fd_set *writePtr = FillSetFromVector(write, inWriteSet, nfds);
	fd_set *exceptPtr = FillSetFromVector(except, inExceptSet, nfds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = timeoutMillis * 1000;

	int toRet = select(nfds + 1, readPtr, writePtr, exceptPtr, &timeout);

	if (toRet > 0)
	{
		FillVectorFromSet(outReadSet, inReadSet, read);
		FillVectorFromSet(outWriteSet, inWriteSet, write);
		FillVectorFromSet(outExceptSet, inExceptSet, except);
	}

	return toRet;
}

#endif
