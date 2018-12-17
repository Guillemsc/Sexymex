#pragma once

#include "Module.h"
#include "Log.h"
#include <vector>
#include <string>

class ModuleLogView: public Module, public LogOutput
{
public:

	// Module virtual methods

	bool init() override;

	bool updateGUI() override;

	// LogOutput virtual methods

	void writeMessage(const std::string &message) override;

private:

	std::vector<std::string> allMessages;
};