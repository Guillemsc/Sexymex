#ifndef NET_H
#define NET_H

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include "Windows.h"
	#include "WinSock2.h"
	#include "Ws2tcpip.h"
	typedef int socklen_t;
	//typedef char* receiveBufer_t;
	#define s_addr S_un.S_addr
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <unistd.h>
	//typedef void* receiveBufer_t;
	typedef int SOCKET;
	const int NO_ERROR = 0;
	const int INVALID_SOCKET = -1;
	const int WSAECONNRESET = ECONNRESET;
	const int WSAEWOULDBLOCK = EAGAIN;
	const int SOCKET_ERROR = -1;
#endif

#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <cassert>

#include "StringUtils.h"
#include "SocketAddress.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "SocketUtil.h"
#include "ByteSwap.h"
#include "MemoryStream.h"
#include "TCPNetworkManager.h"

#endif // MULTIPLAYER_H
