#ifndef SOCKET_ADDRESS_H
#define SOCKET_ADDRESS_H

class SocketAddress
{
public:

	/** Default constructor. */
	SocketAddress()
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_addr.s_addr = INADDR_ANY;
		GetAsSockAddrIn()->sin_port = 0;
	}

	/** Parameterized constructor using a port. */
	SocketAddress(uint16_t inPort)
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_addr.s_addr = INADDR_ANY;
		GetAsSockAddrIn()->sin_port = htons(inPort);
	}

	/** Parameterized constructor using an IP address and a port. */
	SocketAddress(uint32_t inAddress, uint16_t inPort)
	{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetAsSockAddrIn()->sin_addr.s_addr = htonl(inAddress);
		GetAsSockAddrIn()->sin_port = htons(inPort);
	}

	/** Parameterized constructor using an IP addreass and a port. */
	SocketAddress(const std::string &inAddresAndPort);
	
	/** Copy constructor. */
	SocketAddress(const sockaddr &inSockAddr)
	{
		memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
	}

	/**
	 * It returns the size of the sockaddr struct.
	 */
	int GetSize() const { return sizeof(sockaddr); }

	/**
	* It returns the IP:port string in format "xxx.xxx.xxx.xxx:port"
	*/
	std::string GetString() const;

	/**
	* It returns the IP string in format "xxx.xxx.xxx.xxx"
	*/
	std::string GetIPString() const;

	/**
	 * Tells whether or not two addresses are the same.
	 */
	bool operator==(const SocketAddress &s) const
	{
		const auto *sa1 = GetAsSockAddrIn();
		const auto *sa2 = s.GetAsSockAddrIn();
		return
			sa1->sin_family == sa2->sin_family &&
			sa1->sin_addr.s_addr == sa2->sin_addr.s_addr &&
			sa1->sin_port == sa2->sin_port;
	}

	/**
	 * Tells if the address at the lhs is lower than
	 * the one at the rhs of the < operator.
	 */
	bool operator<(const SocketAddress &s) const
	{
		const auto *sa1 = GetAsSockAddrIn();
		const auto *sa2 = s.GetAsSockAddrIn();
		return (sa1->sin_family < sa2->sin_family) ||
			(sa1->sin_family == sa2->sin_family && sa1->sin_addr.s_addr < sa2->sin_addr.s_addr) ||
			(sa1->sin_family == sa2->sin_family && sa1->sin_addr.s_addr == sa2->sin_addr.s_addr && sa1->sin_port < sa2->sin_port);
	}

private:

	// TCPSocket and UDPSockets can access the internals of this class
	friend class TCPSocket;
	friend class UDPSocket;

	sockaddr mSockAddr; /**< The struct containing the socket address. */

	/** It returns the concrete struct sockaddr_in. */
	sockaddr_in* GetAsSockAddrIn()
	{
		return reinterpret_cast<sockaddr_in*>(&mSockAddr);
	}

	/** It returns the concrete struct sockaddr_in (const version). */
	const sockaddr_in* GetAsSockAddrIn() const
	{
		return reinterpret_cast<const sockaddr_in*>(&mSockAddr);
	}
};

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;

#endif // SOCKET_ADDRESS_H
