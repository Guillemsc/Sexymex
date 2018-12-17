#include "Application.h"
#include "Log.h"

// Start the application without the black console in the background
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

Application * App = nullptr;

enum class MainState
{
	Create,
	Init,
	Loop,
	CleanUp,
	Fail,
	Exit
};

int main(int argc, char **argv)
{
	int result = EXIT_FAILURE;

	MainState state = MainState::Create;

	while (state != MainState::Exit)
	{
		switch (state)
		{
		case MainState::Create:
			App = new Application();
			if (App != nullptr) {
				state = MainState::Init;
			} else {
				eLog << "Create failed";
				state = MainState::Fail;
			}
			break;

		case MainState::Init:
			if (App->init()) {
				state = MainState::Loop;
			} else {
				eLog << "Init failed";
				state = MainState::Fail;
			}
			break;

		case MainState::Loop:
			if (App->update()) {
				if (App->wantsToExit()) {
					state = MainState::CleanUp;
				}
			} else {
				eLog << "Loop failed";
				state = MainState::Fail;
			}
			break;

		case MainState::CleanUp:
			if (App->cleanUp()) {
				delete App;
				App = nullptr;
				state = MainState::Exit;
				result = EXIT_SUCCESS;
			} else {
				eLog << "CleanUp failed";
				state = MainState::Fail;
			}
			break;

		case MainState::Fail:
			eLog << "Application failed :-(";
			state = MainState::Exit;
			break;

		case MainState::Exit:
			break;
		}
	}

	return result;
}
