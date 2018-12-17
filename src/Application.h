#pragma once

#include <vector>

// Module declarations
class Module;
class ModuleWindow;
class ModuleTextures;
class ModuleNetworkManager;
class ModuleAgentContainer;
class ModuleLogView;
class ModuleMainMenu;
class ModuleNodeCluster;
class ModuleYellowPages;

class Application
{
public:

	// Constructor and destructor

	Application();

	~Application();


	// Application methods

	bool wantsToExit() const { return wannaExit; }

	void exit() { wannaExit = true; }


	// Application lifetime methods

	bool init();

	bool update();

	bool cleanUp();


	// Called from the window module to destroy/restore GPU objects

	friend ModuleWindow;

	bool invalidateDeviceObjects();

	bool restoreDeviceObjects();


private:

	// Private lifetime methods

	bool doPreUpdate();

	bool doUpdate();

	bool doUpdateGUI();
	
	bool doPostUpdate();


public:

	// Modules
	ModuleWindow *modWindow = nullptr;
	ModuleTextures *modTextures = nullptr;
	ModuleNetworkManager *networkManager = nullptr;
	ModuleAgentContainer *agentContainer = nullptr;
	ModuleLogView *modLogView = nullptr;
	ModuleMainMenu *modMainMenu = nullptr;
	ModuleNodeCluster *modNodeCluster = nullptr;
	ModuleYellowPages *modYellowPages = nullptr;


private:

	// All modules
	std::vector<Module*> modules;

	// Exit flag
	bool wannaExit = false;
};

extern Application* App;
