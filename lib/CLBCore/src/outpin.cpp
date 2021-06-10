#include <Arduino.h>

#include "outpin.h"
#include "errors.h"
#include "utils.h"
#include "settings.h"
#include "processes.h"
#include "messages.h"
#include "mqtt.h"

struct outPinSettings outpinSettings;

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

bool outpinState;

int validateOutPinPosition(float position)
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

int setOutPinState(float position)
{
    if (outPinProcess.status != OUTPIN_OK)
    {
        return JSON_MESSAGE_OUTPIN_NOT_AVAILABLE;
    }

    int result = validateOutPinPosition(position);

    if (result == WORKED_OK)
    {
        if(position <0.5)
        {
            setOutPinUsingActiveHigh(false);
        }
        else
        {
            setOutPinUsingActiveHigh(true);
        }
    }

    return result;
}

struct OutPinCommandItems outpinCommandItems;

boolean validateOutPinCommandString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, OUTPIN_COMMAND_NAME_LENGTH));
}

#define OUTPIN_STATE_COMMAND_OFFSET 0

#define OUTPIN_COMMAND_SIZE = (OUTPIN_STATE_COMMAND_OFFSET+sizeof(float))

struct CommandItem outpinPositionCommandItem = {
    "value",
    "state of outpin (0-1) >=0.5 is on",
    OUTPIN_STATE_COMMAND_OFFSET,
    floatCommand,
    validateFloat0to1,
    noDefaultAvailable};

struct CommandItem *setOutPinItems[] =
    {
        &outpinPositionCommandItem};

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

    float state = getUnalignedFloat(settingBase+OUTPIN_STATE_COMMAND_OFFSET);    

    return setOutPinState(state);
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
        setOutPinItems,
        sizeof(setOutPinItems) / sizeof(struct CommandItem *),
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

    int result = validateOutPinPosition(position);

    if (result == WORKED_OK)
    {
        outpinSettings.OutPinInitialState = position;
        saveSettings();
    }

    return result;
}

struct Command *outpinCommandList[] = {
    &setOutPinCommand,
    &setOutPinInitialStateCommand};

struct CommandItemCollection outpinCommands =
    {
        "Control the outpin",
        outpinCommandList,
        sizeof(outpinCommandList) / sizeof(struct Command *)};

void initOutPin()
{
    outPinProcess.status = OUTPIN_STOPPED;
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
