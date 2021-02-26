#include <Arduino.h>
#include "utils.h"
#include "settings.h"
#include "processes.h"
#include "HullOSCommands.h"
#include "HullOSVariables.h"
#include "HullOSScript.h"
#include "HullOS.h"

struct HullOSSettings hullosSettings;

struct SettingItem hullosEnabled = {
    "HullOS enabled",
    "hullosactive",
    &hullosSettings.hullosEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

boolean validateHullOSCode(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, HULLOS_PROGRAM_SIZE));
}

void setDefaultHullOSCode(void *dest)
{
	strcpy((char *)dest, "WThello world\nRH");
}

struct SettingItem hullosProgramSetting = {
	"hullosCode", "Hullos program", hullosSettings.hullosCode, HULLOS_PROGRAM_SIZE, text, setEmptyString, validateHullOSCode};

struct SettingItem *hullosSettingItemPointers[] =
    {
        &hullosEnabled,
        &hullosProgramSetting};

struct SettingItemCollection hullosSettingItems = {
    "hullos",
    "HullOs active",
    hullosSettingItemPointers,
    sizeof(hullosSettingItemPointers) / sizeof(struct SettingItem *)};

void hullosOff()
{
    hullosProcess.status = HULLOS_STOPPED;
}

void hullosOn()
{
        hullosProcess.status = HULLOS_OK;
        clearVariables();
}

void initHullOS()
{
    hullosProcess.status = HULLOS_STOPPED;
}

void startHullOS()
{
    if (hullosSettings.hullosEnabled)
    {
        hullosProcess.status = HULLOS_OK;
        clearVariables();
    }
}

bool commandsNeedFullSpeed()
{
    return deviceState != EXECUTE_IMMEDIATELY;
}

void updateHullOS()
{
    // If we recieve serial data the program that is running
    // must stop.

    if(hullosProcess.status == HULLOS_STOPPED)
    {
        return;
    }

    while (CharsAvailable())
    {
        byte b = GetRawCh();
        processHullOSSerialByte(b);
    }

    switch (programState)
    {
    case PROGRAM_STOPPED:
    case PROGRAM_PAUSED:
        break;
    case PROGRAM_ACTIVE:
        exeuteProgramStatement();
        break;
    case PROGRAM_AWAITING_DELAY_COMPLETION:
        if (millis() > delayEndTime)
        {
            programState = PROGRAM_ACTIVE;
        }
        break;
    }
}

void stophullos()
{
    hullosProcess.status = HULLOS_STOPPED;
}

bool hullosStatusOK()
{
    return hullosProcess.status == HULLOS_OK;
}

void hullosStatusMessage(char *buffer, int bufferLength)
{
    if (hullosProcess.status == HULLOS_STOPPED)
    {
        snprintf(buffer, bufferLength, "HullOS stopped");
    }
    else
    {
        snprintf(buffer, bufferLength, "HullOS enabled");
    }
}

struct process hullosProcess = {
    "hullos",
    initHullOS,
    startHullOS,
    updateHullOS,
    stophullos,
    hullosStatusOK,
    hullosStatusMessage,
    false,
    0,
    0,
    0,
    NULL,
    (unsigned char *)&hullosSettings, sizeof(HullOSSettings), &hullosSettingItems,
    NULL,
    BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
    NULL,
    NULL,
    NULL,
    NULL, // no command options
    0     // no command options
};
