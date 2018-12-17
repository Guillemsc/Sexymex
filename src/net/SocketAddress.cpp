#include "Net.h"

SocketAddress::SocketAddress(const std::string &inString)
{
	// Parse inString
	const auto pos = inString.find_last_of(':');
	std::string host, service;
	if (pos != std::string::npos)
	{
		host = inString.substr(0, pos);
		service = inString.substr(pos + 1);
	}
	else
	{
		host = inString;
		service = "0"; // default port...
	}

	// Hint to specify we want connections via IPv4
	addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET; // IPv4

	// Get address information into result
	addrinfo *result;
	const int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &result);
	addrinfo *initResult = result;

	if (error == 0)
	{
		// Search the first valid addrinfo
		while (!result->ai_addr && result->ai_next)
		{
			result = result->ai_next;
		}

		// And copy its value
		if (result && result->ai_addr) {
			const sockaddr &addr(*result->ai_addr);
			memcpy(&mSockAddr, &addr, sizeof(addr));
		}
	}

	// Release getaddrinfo results
	freeaddrinfo(initResult);
}

std::string SocketAddress::GetString() const
{
	unsigned char a = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b1;
	unsigned char b = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b2;
	unsigned char c = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b3;
	unsigned char d = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b4;
	int port = ntohs(GetAsSockAddrIn()->sin_port);
	char addressStr[64];
	sprintf_s(addressStr, "%d.%d.%d.%d:%d", a, b, c, d, port);
	std::string address(addressStr);
	return address;
}

std::string SocketAddress::GetIPString() const
{
	unsigned char a = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b1;
	unsigned char b = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b2;
	unsigned char c = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b3;
	unsigned char d = GetAsSockAddrIn()->sin_addr.S_un.S_un_b.s_b4;
	char addressStr[64];
	sprintf_s(addressStr, "%d.%d.%d.%d", a, b, c, d);
	std::string address(addressStr);
	return address;
}
