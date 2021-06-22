#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <Servo.h>
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#endif

#include "errors.h"
#include "utils.h"
#include "settings.h"
#include "servoproc.h"
#include "processes.h"
#include "messages.h"
#include "mqtt.h"

struct ServoSettings servoSettings;

float servoPosition;
unsigned long servoHoldMillisStart;
unsigned long servoHoldTime;
float servoHoldOriginalPosition;
bool servoHoldActive;

Servo *servo = NULL;

void setDefaultServoOutputPin(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 12;
}

void setDefaultServoInitialPosition(void *dest)
{
    double *destDouble = (double *)dest;
    *destDouble = 0.5;
}

struct SettingItem ServoOutputPinSetting = {
    "Servo output Pin",
    "servooutputpin",
    &servoSettings.ServoOutputPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultServoOutputPin,
    validateInt};

struct SettingItem ServoInitialAngleSetting = {
    "Servo initial angle",
    "servoinitialangle",
    &servoSettings.ServoInitialPosition,
    NUMBER_INPUT_LENGTH,
    floatValue,
    setDefaultServoInitialPosition,
    validateFloat0to1};

struct SettingItem ServoEnabled = {
    "Servo enabled",
    "servoactive",
    &servoSettings.ServoEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

struct SettingItem *ServoSettingItemPointers[] =
    {
        &ServoOutputPinSetting,
        &ServoInitialAngleSetting,
        &ServoEnabled};

struct SettingItemCollection ServoSettingItems = {
    "Servo",
    "Pin assignment and enable/disable for Servo output",
    ServoSettingItemPointers,
    sizeof(ServoSettingItemPointers) / sizeof(struct SettingItem *)};

int validateServoPosition(float position)
{
    if (position < 0)
    {
        return JSON_MESSAGE_SERVO_VALUE_TOO_LOW;
    }

    if (position > 1)
    {
        return JSON_MESSAGE_SERVO_VALUE_TOO_HIGH;
    }

    return WORKED_OK;
}

int setServoPosition(float position)
{
    if (ServoProcess.status != SERVO_OK)
    {
        return JSON_MESSAGE_SERVO_NOT_AVAILABLE;
    }

    int result = validateServoPosition(position);

    if (result == WORKED_OK)
    {
        int servoDriveValue =(int)(position*180);
        servoPosition = position;
        servo->write(servoDriveValue);
    }
    return result;
}

struct ServoCommandItems servoCommandItems;

boolean validateServoCommandString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, SERVO_COMMAND_NAME_LENGTH));
}

#define SERVO_POSITION_COMMAND_OFFSET 0
#define SERVO_PULSE_LENGTH_OFFSET COMMAND_OPTION_AREA_START

#define SERVO_COMMAND_SIZE = (SERVO_POSITION_COMMAND_OFFSET+sizeof(float))

struct CommandItem servoPositionCommandItem = {
    "value",
    "position of servo (0-1)",
    SERVO_POSITION_COMMAND_OFFSET,
    floatCommand,
    validateFloat0to1,
    noDefaultAvailable};

// ************************************* Set servo position

struct CommandItem *setServoPositionItems[] =
    {
        &servoPositionCommandItem
    };

int doSetServoPositionCommand(char * destination, unsigned char * settingBase);

struct Command setServoPositionCommand
{
    "setservopos",
        "Sets the position of the servo",
        setServoPositionItems,
        sizeof(setServoPositionItems) / sizeof(struct CommandItem *),
        doSetServoPositionCommand
};

int doSetServoPositionCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("servo", &setServoPositionCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    float newPosition = getUnalignedFloat(settingBase+SERVO_POSITION_COMMAND_OFFSET);  

    if (ServoProcess.status != SERVO_OK)
    {
        return JSON_MESSAGE_SERVO_NOT_AVAILABLE;
    }

    if(newPosition == servoPosition)
    {
        return WORKED_OK;
    }

    int result = setServoPosition(newPosition);

    if(result != WORKED_OK){
        return result;
    }

    servoHoldActive=false;

    return WORKED_OK;
}

// ************************************ Pulse servo position

bool validateServoPulseLen(void *dest, const char *newValueStr)
{
	float value;

	if (!validateFloat(&value, newValueStr))
	{
		return false;
	}

	if (value < 0 || value > SERVO_MAX_HOLD_TIME_SECS)
	{
		return false;
	}

    putUnalignedFloat(value,(unsigned char *)dest);
	return true;
}

struct CommandItem servoPulseLenCommandItem = {
    "pulselen",
    "length of the pulse in secs",
    SERVO_PULSE_LENGTH_OFFSET,
    floatCommand,
    validateServoPulseLen,
    noDefaultAvailable};

struct CommandItem *pulseServoPositionItems[] =
    {
        &servoPositionCommandItem,
        &servoPulseLenCommandItem};

int doPulseServoPositionCommand(char * destination, unsigned char * settingBase);

struct Command pulseServoPositionCommand
{
    "pulseservopos",
        "Pulses the position of the servo",
        pulseServoPositionItems,
        sizeof(pulseServoPositionItems) / sizeof(struct CommandItem *),
        doPulseServoPositionCommand
};

int doPulseServoPositionCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("servo", &pulseServoPositionCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    if (ServoProcess.status != SERVO_OK)
    {
        return JSON_MESSAGE_SERVO_NOT_AVAILABLE;
    }

    float newPosition = getUnalignedFloat(settingBase+SERVO_POSITION_COMMAND_OFFSET);  

    if(newPosition == servoPosition)
    {
        return WORKED_OK;
    }

    float oldServoPosition = servoPosition;

    int result = setServoPosition(newPosition);

    if(result != WORKED_OK){
        return result;
    }

    float hold = getUnalignedFloat(settingBase+SERVO_PULSE_LENGTH_OFFSET); 

    servoHoldMillisStart = millis();
    servoHoldTime = hold*1000;
    servoHoldOriginalPosition = oldServoPosition;
    servoHoldActive=true;

    return WORKED_OK;
}

struct CommandItem servoInitialPositionCommandItem = {
    "initialservoPosition",
    "initialposition of servo (0-1)",
    SERVO_POSITION_COMMAND_OFFSET,
    floatCommand,
    validateFloat0to1};

struct CommandItem *setServoInitialPositionItems[] =
    {
        &servoInitialPositionCommandItem};

int doSetServoInitialPositionCommand(char * destination, unsigned char * settingBase);

struct Command setServoInitialPositionCommand
{
    "setinitservopos",
        "Sets the initial position of the servo",
        setServoPositionItems,
        sizeof(setServoPositionItems) / sizeof(struct CommandItem *),
        doSetServoInitialPositionCommand
};

int doSetServoInitialPositionCommand(char * destination, unsigned char * settingBase)
{
    if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("servo", &setServoInitialPositionCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

    float position = getUnalignedFloat(settingBase+SERVO_POSITION_COMMAND_OFFSET);    

    int result = validateServoPosition(position);

    if (result == WORKED_OK)
    {
        servoSettings.ServoInitialPosition = position;
        saveSettings();
    }

    return result;
}

struct Command *servoCommandList[] = {
    &setServoPositionCommand,
    &pulseServoPositionCommand,
    &setServoInitialPositionCommand};

struct CommandItemCollection servoCommands =
    {
        "Control the servo",
        servoCommandList,
        sizeof(servoCommandList) / sizeof(struct Command *)};

void initServo()
{
    ServoProcess.status = SERVO_STOPPED;
}

void startServo()
{
    if (servoSettings.ServoEnabled)
    {
        if (servo == NULL)
        {
            servo = new Servo();
            servo->attach(servoSettings.ServoOutputPin,SERVO_MIN, SERVO_MAX);
            ServoProcess.status = SERVO_OK;
            setServoPosition(servoSettings.ServoInitialPosition);
        }
    }
    else
    {
        ServoProcess.status = SERVO_STOPPED;
    }
}

void updateServo()
{
    // need to handle the timeout of the hold function
    // Don't do anything if the pin is turned off
    if (ServoProcess.status != SERVO_OK)
    {
        return;
    }

    // Don't do anything if there is not a hold active
    if(!servoHoldActive){
        return;
    }

    // Get the time since the hold started
    unsigned long elapsedTime = ulongDiff(millis(),servoHoldMillisStart);

    // Quit if the hold is not over yet
    if(elapsedTime<servoHoldTime){
        return;
    }

    // have reached the end of the hold

    // Put the servo back
    setServoPosition(servoHoldOriginalPosition);

    // turn off the hold
    servoHoldActive = false;
}

void stopServo()
{
    ServoProcess.status = SERVO_STOPPED;
}

bool servoStatusOK()
{
    return ServoProcess.status == SERVO_OK;
}

void ServoStatusMessage(char *buffer, int bufferLength)
{
    if (ServoProcess.status == SERVO_OK)
        snprintf(buffer, bufferLength, "Servo angle: %f", servoPosition);
    else
        snprintf(buffer, bufferLength, "Servo off");
}

struct process ServoProcess = {
    "servo",
    initServo,
    startServo,
    updateServo,
    stopServo,
    servoStatusOK,
    ServoStatusMessage,
    false,
    0,
    0,
    0,
    NULL,
    (unsigned char *)&servoSettings, sizeof(servoSettings), &ServoSettingItems,
    &servoCommands,
    BOOT_PROCESS + ACTIVE_PROCESS,
    NULL,
    NULL,
    NULL
    };
