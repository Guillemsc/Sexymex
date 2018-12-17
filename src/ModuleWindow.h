#pragma once

#include "Module.h"

class ModuleWindow : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool preUpdate() override;

	bool postUpdate() override;

	bool cleanUp() override;
};