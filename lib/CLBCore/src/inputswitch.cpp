#include <Arduino.h>

#include "utils.h"
#include "inputswitch.h"
#include "settings.h"

// Input switch sensor

struct InputSwitchSettings inputSwitchSettings;

void setDefaultInputPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 5;
}

void setDefaultInputGroundPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = -1;
}

struct SettingItem inputSwitchPinNo = {
	"Switch Input Pin",
	"switchinputpin",
	&inputSwitchSettings.inputPin,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultInputPinNo,
	validateInt};

struct SettingItem inputSwitchGroundPinNo = {
	"Switch Input Ground Pin (-1 for not used)",
	"switchinputgroundpin",
	&inputSwitchSettings.groundPin,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultInputGroundPinNo,
	validateInt};

struct SettingItem inputSwitchActiveLow = {
	"Switch Input Active Low",
	"switchinputactivelow",
	&inputSwitchSettings.activeLow,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

void setDefaultInputSwitchFunction(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = INPUT_SWITCH_UNUSED_SETTING;
}

boolean validateInputSwitchFunction(void *dest, const char *newValueStr)
{
	int config;

	bool validConfig = validateInt(&config,newValueStr);

	if(!validConfig) return false;

	if(config<0||config>1) return false;

	int * intDest = (int*) dest;

	*intDest = config;

	return true;
}

struct SettingItem inputSwitchFunctionConfig = {
	"Input switch function (0-unused) (1-wifi config)",
	"switchinputwificonfig",
	&inputSwitchSettings.function,
	ONOFF_INPUT_LENGTH,
	integerValue,
	setDefaultInputSwitchFunction,
	validateInputSwitchFunction};

struct SettingItem *inputSwitchSettingItemPointers[] =
	{
		&inputSwitchPinNo,
		&inputSwitchGroundPinNo,
		&inputSwitchActiveLow,
		&inputSwitchFunctionConfig};

struct SettingItemCollection inputSwitchSettingItems = {
	"Input switch",
	"Input switch configuration",
	inputSwitchSettingItemPointers,
	sizeof(inputSwitchSettingItemPointers) / sizeof(struct SettingItem *)};

int lastInputValue;
long inputDebounceStartTime;

boolean switchValue;

boolean getInputSwitchValue()
{
	return switchValue;
}

unsigned long millisAtLastInputChange;

// number of milliseconds that the signal must be stable
// before we read it

#define INPUT_DEBOUNCE_TIME 10

void initInputSwitch()
{
	inputSwitchProcess.status = INPUT_SWITCH_STOPPED;
}

void enableInputSwitch()
{
	pinMode(inputSwitchSettings.inputPin, INPUT_PULLUP);

	if (inputSwitchSettings.groundPin != -1)
	{
		pinMode(inputSwitchSettings.groundPin, OUTPUT);
		digitalWrite(inputSwitchSettings.groundPin, LOW);
	}
	inputSwitchProcess.status = INPUT_SWITCH_OK;
}

void startInputSwitch()
{
	switch(inputSwitchSettings.function)
	{ 
		case INPUT_SWITCH_UNUSED_SETTING:
		inputSwitchProcess.status = INPUT_SWITCH_STOPPED;
		break;

		case INPUT_SWITCH_WIFI_SETTING:
		enableInputSwitch();
		break;
	}
}

void updateInputSwitch()
{
	if(inputSwitchProcess.status == INPUT_SWITCH_STOPPED)
	{
		return;
	}

	int newInputValue = digitalRead(inputSwitchSettings.inputPin);

	if (newInputValue == lastInputValue)
	{
		inputDebounceStartTime = millis();
	}
	else
	{
		long currentMillis = millis();
		long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

		if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
		{
			if (newInputValue)
				switchValue = !inputSwitchSettings.activeLow;
			else
				switchValue = inputSwitchSettings.activeLow;
			lastInputValue = newInputValue;
		}
	}
}

void stopInputSwitch()
{
	inputSwitchProcess.status = INPUT_SWITCH_STOPPED;
}

bool inputSwitchStatusOK()
{
	return inputSwitchProcess.status == INPUT_SWITCH_OK;
}


void inputSwitchStatusMessage(char *buffer, int bufferLength)
{
	if(inputSwitchProcess.status == INPUT_SWITCH_STOPPED)
	{
		snprintf(buffer, bufferLength, "Input switch stopped");
		return;
	}

	if (switchValue)
		snprintf(buffer, bufferLength, "Input switch pressed");
	else
		snprintf(buffer, bufferLength, "Input switch released");
}

boolean readInputSwitch()
{
	int lastInputValue = digitalRead(inputSwitchSettings.activeLow);
	long inputDebounceStartTime = millis();

	while (true)
	{
		int newInputValue = digitalRead(inputSwitchSettings.activeLow);

		if (newInputValue != lastInputValue)
		{
			inputDebounceStartTime = millis();
			lastInputValue = newInputValue;
		}
		else
		{
			long currentMillis = millis();
			long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

			if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
			{
				if (newInputValue)
					return !inputSwitchSettings.activeLow;
				else
					return inputSwitchSettings.activeLow;
			}
		}
	}
}

struct process inputSwitchProcess = {
	"inputswitch",
	initInputSwitch,
	startInputSwitch,
	updateInputSwitch,
	stopInputSwitch,
	inputSwitchStatusOK,
	inputSwitchStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&inputSwitchSettings, sizeof(InputSwitchSettings), &inputSwitchSettingItems,
	NULL,
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL,
    NULL,     // no command options
    0         // no command options 
};
