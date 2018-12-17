#pragma once

#include "Module.h"
#include "AgentLocation.h"
#include "net/Net.h"
#include <map>

class IDatabaseGateway;

class ModuleYellowPages : public Module, public TCPNetworkManagerDelegate
{
public:

	// Module virtual methods

	bool init() override;

	bool start() override;

	bool update() override;

	bool updateGUI() override;

	bool stop() override;


	// TCPNetworkManagerDelegate virtual methods

	void OnAccepted(TCPSocketPtr socket) override;

	void OnPacketReceived(TCPSocketPtr socket, InputMemoryStream &stream) override;

	void OnDisconnected(TCPSocketPtr socket) override;

private:

	bool startService();

	void stopService();

	int state = 0;

	std::map<uint16_t, std::list<AgentLocation> > _mccByItem; /**< MCCs accessed by item id. */
};