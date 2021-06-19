#include <Arduino.h>

#include "outpin.h"
#include "errors.h"
#include "utils.h"
#include "settings.h"
#include "processes.h"
#include "messages.h"
#include "mqtt.h"

struct outPinSettings outpinSettings;

unsigned long outPinHoldMillisStart;
unsigned long outPinHoldTime;
bool outpinHoldOriginalState;
bool outpinHoldActive;
bool outpinState;

void setDefaultOutPinOutputPin(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 16;
}

struct SettingItem OutPinOutputPinSetting = {
    "OutPin output Pin",
    "outpin",
    &outpinSettings.OutPinOutputPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultOutPinOutputPin,
    validateInt};

struct SettingItem OutPinActiveHighSetting = {
    "OutPin active high",
    "outpinactivehigh",
    &outpinSettings.OutPinActiveHigh,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setTrue,
    validateYesNo};

struct SettingItem OutPinEnabled = {
    "OutPin enabled",
    "outpinactive",
    &outpinSettings.OutPinEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

struct SettingItem OutPinInitialState = {
    "OutPin initial state",
    "outpininitialhigh",
    &outpinSettings.OutPinInitialState,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

struct SettingItem *OutPinSettingItemPointers[] =
    {
        &OutPinOutputPinSetting,
        &OutPinActiveHighSetting,
        &OutPinEnabled,
        &OutPinInitialState};

struct SettingItemCollection OutPinSettingItems = {
    "OutPin",
    "Pin assignment, active high and enable/disable for output pin",
    OutPinSettingItemPointers,
    sizeof(OutPinSettingItemPointers) / sizeof(struct SettingItem *)};

int validateOutPinState(float position)
{
    return WORKED_OK;
}

void setOutPinUsingActiveHigh(bool value){

    outpinState = value;

    if(outpinSettings.OutPinActiveHigh)
    {
        if(value)
            digitalWrite(outpinSettings.OutPinOutputPin, 1);
        else
            digitalWrite(outpinSettings.OutPinOutputPin, 0);
    }
    else 
    {
        if(value)
            digitalWrite(outpinSettings.OutPinOutputPin, 0);
        else
            digitalWrite(outpinSettings.OutPinOutputPin, 1);
    }
}

bool outPinStateHigh(float stateFloat)
{
    return stateFloat >= 0.5;
}

struct OutPinCommandItems outpinCommandItems;

boolean validateOutPinCommandString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, OUTPIN_COMMAND_NAME_LENGTH));
}

#define OUTPIN_STATE_COMMAND_OFFSET 0
#define OUTPIN_PULSE_LENGTH_OFFSET COMMAND_OPTION_AREA_START

#define OUTPIN_COMMAND_SIZE = (OUTPIN_HOLD_TIME_OFFSET+sizeof(float))

struct CommandItem outpinStatusCommandItem = {
    "value",
    "state of outpin (0-1) >=0.5 is on",
    OUTPIN_STATE_COMMAND_OFFSET,
    floatCommand,
    validateFloat0to1,
    noDefaultAvailable};

// ************************************* Set pin state

struct CommandItem *setOutPinItems[] =
    {
        &outpinStatusCommandItem
    };

int doSetOutPinCommand(char * destination, unsigned char * settingBase);

struct Command setOutPinCommand
{
    "setoutpinstate",
        "Sets the state of the outpin",
        setOutPinItems,
        sizeof(setOutPinItems) / sizeof(struct CommandItem *),
        doSetOutPinCommand
};

int doSetOutPinCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("outpin", &setOutPinCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    if (outPinProcess.status != OUTPIN_OK)
    {
        return JSON_MESSAGE_OUTPIN_NOT_AVAILABLE;
    }

    float stateValue = getUnalignedFloat(settingBase+OUTPIN_STATE_COMMAND_OFFSET); 

    bool newState = outPinStateHigh(stateValue);

    // Have we changed state?

    if(newState == outpinState)
    {
        // return if we have - no need to change anything
        return WORKED_OK;
    }

    setOutPinUsingActiveHigh(newState);

    return WORKED_OK;
}


// ************************************* Pulse pin state

boolean validateOutpinPulseLen(void *dest, const char *newValueStr)
{
	float value;

	if (!validateFloat(&value, newValueStr))
	{
		return false;
	}

	if (value < 0 || value > OUTPIN_MAX_HOLD_TIME_SECS)
	{
		return false;
	}

    putUnalignedFloat(value,(unsigned char *)dest);
	return true;
}

struct CommandItem outpinPulseLenCommandItem = {
    "pulselen",
    "length of the pulse in secs",
    OUTPIN_PULSE_LENGTH_OFFSET,
    floatCommand,
    validateOutpinPulseLen,
    setDefaultFloatZero};

struct CommandItem *pulseOutPinItems[] =
    {
        &outpinStatusCommandItem,
        &outpinPulseLenCommandItem
    };

int doPulseOutPinCommand(char * destination, unsigned char * settingBase);

struct Command pulseOutPinCommand
{
    "pulseoutpinstate",
        "Pulse the outpin",
        pulseOutPinItems,
        sizeof(pulseOutPinItems) / sizeof(struct CommandItem *),
        doPulseOutPinCommand
};

int doPulseOutPinCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("outpin", &pulseOutPinCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    if (outPinProcess.status != OUTPIN_OK)
    {
        return JSON_MESSAGE_OUTPIN_NOT_AVAILABLE;
    }

    float stateValue = getUnalignedFloat(settingBase+OUTPIN_STATE_COMMAND_OFFSET); 

    bool newState = outPinStateHigh(stateValue);

    // Have we changed state?

    if(newState == outpinState)
    {
        // return if we have - no need to change anything
        return WORKED_OK;
    }

    float hold = getUnalignedFloat(settingBase+OUTPIN_PULSE_LENGTH_OFFSET);  

    outPinHoldMillisStart = millis();
    outPinHoldTime = hold*1000;
    // haven't set the pin to the new state yet - record the current state
    outpinHoldOriginalState = outpinState;
    outpinHoldActive=true;

    setOutPinUsingActiveHigh(newState);

    return WORKED_OK;
}

struct CommandItem outpinInitialPositionCommandItem = {
    "initialoutpinstate",
    "initial state of outpin (0-1)",
    OUTPIN_STATE_COMMAND_OFFSET,
    floatCommand,
    validateFloat0to1};

struct CommandItem *setOutPinInitialPositionItems[] =
    {
        &outpinInitialPositionCommandItem};

int doSetOutPinInitialStateCommand(char * destination, unsigned char * settingBase);

struct Command setOutPinInitialStateCommand
{
    "setinitoutpinstate",
        "Sets the initial state of the outpin",
        setOutPinInitialPositionItems,
        sizeof(setOutPinInitialPositionItems) / sizeof(struct CommandItem *),
        doSetOutPinInitialStateCommand
};

int doSetOutPinInitialStateCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("outpin", &setOutPinInitialStateCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    float position = getUnalignedFloat(settingBase+OUTPIN_STATE_COMMAND_OFFSET);    

    int result = validateOutPinState(position);

    if (result == WORKED_OK)
    {
        outpinSettings.OutPinInitialState = position;
        saveSettings();
    }

    return result;
}

struct Command *outpinCommandList[] = {
    &setOutPinCommand,
    &pulseOutPinCommand,
    &setOutPinInitialStateCommand};

struct CommandItemCollection outpinCommands =
    {
        "Control the outpin",
        outpinCommandList,
        sizeof(outpinCommandList) / sizeof(struct Command *)};

void initOutPin()
{
    outPinProcess.status = OUTPIN_STOPPED;
    outpinHoldActive = false;
}

void startOutPin()
{
    if (outpinSettings.OutPinEnabled)
    {
        outPinProcess.status = OUTPIN_OK;
		pinMode(outpinSettings.OutPinOutputPin, OUTPUT);
        setOutPinUsingActiveHigh(outpinSettings.OutPinInitialState);
    }
    else
    {
        outPinProcess.status = OUTPIN_STOPPED;
    }
}

void updateOutPin()
{
    // need to handle the timeout of the hold function
    // Don't do anything if the pin is turned off
    if (outPinProcess.status != OUTPIN_OK)
    {
        return;
    }

    // Don't do anything if there is not a hold active
    if(!outpinHoldActive){
        return;
    }

    // Get the time since the hold started
    unsigned long elapsedTime = ulongDiff(millis(),outPinHoldMillisStart);

    // Quit if the hold is not over yet
    if(elapsedTime<outPinHoldTime){
        return;
    }

    // have reached the end of the hold

    // Put the pin back
    setOutPinUsingActiveHigh(outpinHoldOriginalState);

    // turn off the hold
    outpinHoldActive = false;
}

void stopOutPin()
{
    outPinProcess.status = OUTPIN_STOPPED;
}

bool outpinStatusOK()
{
    return outPinProcess.status == OUTPIN_OK;
}

void OutPinStatusMessage(char *buffer, int bufferLength)
{
    if (outPinProcess.status == OUTPIN_OK){
        if(outpinState)
        snprintf(buffer, bufferLength, "OutPin high");
        else 
        snprintf(buffer, bufferLength, "OutPin low");
    }
    else
        snprintf(buffer, bufferLength, "OutPin off");
}

struct process outPinProcess = {
    "outpin",
    initOutPin,
    startOutPin,
    updateOutPin,
    stopOutPin,
    outpinStatusOK,
    OutPinStatusMessage,
    false,
    0,
    0,
    0,
    NULL,
    (unsigned char *)&outpinSettings, sizeof(outpinSettings), &OutPinSettingItems,
    &outpinCommands,
    BOOT_PROCESS + ACTIVE_PROCESS,
    NULL,
    NULL,
    NULL
    };
