#pragma once

#define HULLOS_OK 400
#define HULLOS_STOPPED 401

struct HullOSSettings {
	bool hullosEnabled;
};

void hullosOff();

void hullosOn();

extern struct HullOSSettings hullosSettings;

extern struct SettingItemCollection hullosSettingItems;

extern struct process hullosProcess;
