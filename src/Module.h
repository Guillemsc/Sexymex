#pragma once

class Module
{
public:

	// Constructor and destructor

	Module() { }

	virtual ~Module() { }


	// Common functions

	bool isEnabled() const { return active; }

	void setEnabled(bool a) {
		bool prevA = active;
		active = a;
		if (!prevA && a) {
			start();
		} else if (prevA && !a) {
			stop();
		}
	}


	// Virtual functions

	virtual bool init() { return true; }

	virtual bool start() { return true; }

	virtual bool preUpdate() { return true; }

	virtual bool update() { return true; }

	virtual bool updateGUI() { return true; }

	virtual bool postUpdate() { return true; }

	virtual bool stop() { return true;  }

	virtual bool cleanUp() { return true;  }

	virtual bool invalidateDeviceObjects() { return true; }

	virtual bool restoreDeviceObjects() { return true; }

private:

	bool active = false;
};

