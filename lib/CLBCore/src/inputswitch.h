#pragma once

#include "processes.h"

#define INPUT_SWITCH_OK 300
#define INPUT_SWITCH_STOPPED 301

#define INPUT_SWITCH_UNUSED_SETTING 0
#define INPUT_SWITCH_WIFI_SETTING 1

boolean getInputSwitchValue();
boolean readInputSwitch();
boolean inputSwitchConfigsWifi();

struct InputSwitchSettings 
{
    int inputPin;
    int groundPin;
    bool activeLow;
    int function;
};

extern struct InputSwitchSettings inputSwitchSettings;
extern struct process inputSwitchProcess;

void inputSwitchStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);
