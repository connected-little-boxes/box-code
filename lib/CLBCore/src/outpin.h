#pragma once

#define OUTPIN_OK 1500
#define OUTPIN_STOPPED 1501
#define OUTPIN_COMMAND_NAME_LENGTH 40

struct outPinSettings {
	int OutPinOutputPin;
	bool OutPinInitialState;
	bool OutPinActiveHigh;
	bool OutPinEnabled;
};

int setOutPinState(float state);

int startOutPin(struct process * outPinProcess);

int updateOutPin(struct process * outPinProcess);

int stopOutPin(struct process * OutPinProcess);

void OutPinStatusMessage(struct process * outPinProcess, char * buffer, int bufferLength);

extern struct outPinSettings outPinSettings;

extern struct SettingItemCollection OutPinSettingItems;

struct OutPinCommandItems
{
	int position;
};

extern struct OutPinCommandItems outpinCommandItems;

extern struct process outPinProcess;
