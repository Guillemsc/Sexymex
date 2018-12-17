#pragma once

#include "Module.h"
#include "net/Net.h"

class ModuleNetworkManager : public Module, public TCPNetworkManager
{
public:

	// Module virtual functions

	bool init() override;

	bool preUpdate() override;

	bool postUpdate() override;

	bool stop() override;

	bool cleanUp() override;

public:

	void drawInfoGUI();
};
