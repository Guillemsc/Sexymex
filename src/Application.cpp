#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleTextures.h"
#include "ModuleNetworkManager.h"
#include "ModuleAgentContainer.h"
#include "ModuleMainMenu.h"
#include "ModuleNodeCluster.h"
#include "ModuleYellowPages.h"
#include "ModuleLogView.h"

#define ADD_MODULE(ModuleClass, moduleAttribute) \
	moduleAttribute = new ModuleClass(); \
	modules.push_back(moduleAttribute);

static Application *g_Instance = nullptr;

Application::Application()
{
	// Create modules
	ADD_MODULE(ModuleWindow, modWindow);
	ADD_MODULE(ModuleLogView, modLogView);
	ADD_MODULE(ModuleTextures, modTextures);
	ADD_MODULE(ModuleNetworkManager, networkManager);
	ADD_MODULE(ModuleAgentContainer, agentContainer);
	ADD_MODULE(ModuleMainMenu, modMainMenu);
	ADD_MODULE(ModuleNodeCluster, modNodeCluster);
	ADD_MODULE(ModuleYellowPages, modYellowPages);
}


Application::~Application()
{
	// Destroy modules
	for (auto module : modules) {
		delete module;
	}
}


bool Application::init()
{
	// Initialize modules
	for (auto module : modules) {
		module->init();
	}

	// Set active modules (calls start() on them)
	modWindow->setEnabled(true);
	modTextures->setEnabled(true);
	networkManager->setEnabled(true);
	modMainMenu->setEnabled(true);
	modLogView->setEnabled(true);

	return true;
}

bool Application::update()
{
	if (doPreUpdate() == false) return false;

	if (doUpdate() == false) return false;

	if (doUpdateGUI() == false) return false;

	if (doPostUpdate() == false) return false;

	return true;
}

bool Application::cleanUp()
{
	// Stop modules in reverse order
	for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
		auto module = *it;
		module->setEnabled(false);
	}

	// Cleanup in reverse order
	for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
		auto module = *it;
		if (module->cleanUp() == false) {
			return false;
		}
	}

	return true;
}

bool Application::invalidateDeviceObjects()
{
	for (auto module : modules)
	{
		if (module->invalidateDeviceObjects() == false) return false;
	}
	return true;
}

bool Application::restoreDeviceObjects()
{
	for (auto module : modules)
	{
		if (module->restoreDeviceObjects() == false) return false;
	}
	return true;
}

bool Application::doPreUpdate()
{
	for (auto module : modules)
	{
		if (module->isEnabled() == false) continue;
		
		if (module->preUpdate() == false) return false;
	}
	return true;
}

bool Application::doUpdate()
{
	for (auto module : modules)
	{
		if (module->isEnabled() == false) continue;

		if (module->update() == false) return false;
	}
	return true;
}

bool Application::doUpdateGUI()
{
	for (auto module : modules)
	{
		if (module->isEnabled() == false) continue;

		if (module->updateGUI() == false) return false;
	}
	return true;
}

bool Application::doPostUpdate()
{
	for (auto module : modules)
	{
		if (module->isEnabled() == false) continue;

		if (module->postUpdate() == false) return false;
	}
	return true;
}
