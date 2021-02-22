#pragma once

#define HULLOS_OK 400
#define HULLOS_STOPPED 401

#define HULLOS_PROGRAM_SIZE 100

struct HullOSSettings {
	bool hullosEnabled;
	unsigned char hullosCode[HULLOS_PROGRAM_SIZE];
};

void hullosOff();

void hullosOn();

extern struct HullOSSettings hullosSettings;

extern struct SettingItemCollection hullosSettingItems;

extern struct process hullosProcess;

#define STORED_PROGRAM_OFFSET 20


