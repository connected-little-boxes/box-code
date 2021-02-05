#include "buttonSensor.h"
#include "debug.h"
#include "settings.h"
#include "controller.h"
#include "sensors.h"
#include "mqtt.h"
#include "pixels.h"

struct ButtonSensorSettings buttonSensorSettings;

void setDefaultButtonInputPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 14;
}

void setDefaultButtonInputGroundPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 16;
}

struct SettingItem buttonSensorPinNo = {
	"Push Button Input Pin",
	"buttoninputpin",
	&buttonSensorSettings.buttonSensorInputPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultButtonInputPinNo,
	validateInt};

struct SettingItem buttonSensorGroundPinNo = {
	"Push Button Input Ground Pin (-1 for not used)",
	"buttoninputgroundpin",
	&buttonSensorSettings.buttonGroundPin,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultButtonInputGroundPinNo,
	validateInt};

struct SettingItem buttonSensorFitted = {
	"Push Button Input Fitted",
	"pushbuttonfitted",
	&buttonSensorSettings.buttonSensorFitted,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem *buttonSensorSettingItemPointers[] =
	{
		&buttonSensorPinNo,
		&buttonSensorGroundPinNo,
		&buttonSensorFitted};

struct SettingItemCollection buttonSensorSettingItems = {
	"Push Button Switch",
	"Push Button Switch configuration",
	buttonSensorSettingItemPointers,
	sizeof(buttonSensorSettingItemPointers) / sizeof(struct SettingItem *)};

struct sensorEventBinder ButtonSensorListenerFunctions[] = {
	{"pressed", BUTTONSENSOR_BUTTON_PRESSED_MASK_BIT},
	{"released", BUTTONSENSOR_BUTTON_RELEASED_MASK_BIT},
	{"changed", BUTTONSENSOR_SEND_ON_CHANGE_MASK_BIT}};

int lastButtonInputValue;
long buttonInputDebounceStartTime;
unsigned long millisAtLastButtonInputChange;

void readButtonSensor(struct buttonSensorReading *buttonSensorActiveReading)
{
	int newInputValue = digitalRead(buttonSensorSettings.buttonSensorInputPinNo);

	if (newInputValue == lastButtonInputValue)
	{
		buttonInputDebounceStartTime = millis();
	}
	else
	{
		long currentMillis = millis();
		long millisSinceChange = ulongDiff(currentMillis, buttonInputDebounceStartTime);

		if (++millisSinceChange > BUTTON_INPUT_DEBOUNCE_TIME)
		{
			if (newInputValue)
			{
				buttonSensorActiveReading->pressed = false;
			}
			else
			{
				buttonSensorActiveReading->pressed = true;
			}
			lastButtonInputValue = newInputValue;
		}
	}
}

bool updateButtonSensor()
{
	if (!buttonSensor.beingUpdated)
	{
		return false;
	}

	struct buttonSensorReading *buttonSensoractiveReading =
		(struct buttonSensorReading *)buttonSensor.activeReading;

	bool previousReading = buttonSensoractiveReading->pressed;

	readButtonSensor(buttonSensoractiveReading);

	buttonSensor.millisAtLastReading = millis();

	if (buttonSensoractiveReading->pressed == previousReading)
	{
		// we have read the button but the value has not changed
		// just return that we worked OK
		return true;
	}

	// if we get here we have a change in the reading
	// see who wants to know

	sensorListener *pos = buttonSensor.listeners;

	while (pos != NULL)
	{
		if (pos->config->sendOptionMask & BUTTONSENSOR_SEND_ON_CHANGE_MASK_BIT)
		{
			// send on change - so send for this listener
			pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
			pos->lastReadingMillis = buttonSensor.millisAtLastReading;
			// move on to the next one
			pos = pos->nextMessageListener;
			continue;
		}

		if (pos->config->sendOptionMask & BUTTONSENSOR_BUTTON_PRESSED_MASK_BIT)
		{
			// send on pressed - is the button pressed now?
			if (buttonSensoractiveReading->pressed)
			{
				pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
				pos->lastReadingMillis = buttonSensor.millisAtLastReading;
				// move on to the next one
				pos = pos->nextMessageListener;
				continue;
			}
		}

		if (pos->config->sendOptionMask & BUTTONSENSOR_BUTTON_RELEASED_MASK_BIT)
		{
			// send on pressed - is the button pressed now?
			if (!buttonSensoractiveReading->pressed)
			{
				pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
				pos->lastReadingMillis = buttonSensor.millisAtLastReading;
				// move on to the next one
				pos = pos->nextMessageListener;
				continue;
			}
		}

		// move on to the next one
		pos = pos->nextMessageListener;
	}
	return true;
}

void startbuttonSensor()
{
	if (buttonSensor.activeReading == NULL)
	{
		buttonSensor.activeReading = new buttonSensorReading();
	}

	if (!buttonSensorSettings.buttonSensorFitted)
	{
		buttonSensor.status = BUTTONSENSOR_NOT_FITTED;
	}
	else
	{
		pinMode(buttonSensorSettings.buttonSensorInputPinNo, INPUT_PULLUP);

		if (buttonSensorSettings.buttonGroundPin != -1)
		{
			pinMode(buttonSensorSettings.buttonGroundPin, OUTPUT);
			digitalWrite(buttonSensorSettings.buttonGroundPin, LOW);
		}

		buttonSensor.status = SENSOR_OK;
	}
}

void stopButtonSensor()
{
}

void updateButtonSensorReading()
{
	switch (buttonSensor.status)
	{
	case SENSOR_OK:
		updateButtonSensor();
		break;

	case BUTTONSENSOR_STOPPED:
		break;
	}
}

void startButtonSensorReading()
{
}

void addButtonSensorReading(char *jsonBuffer, int jsonBufferSize)
{
	struct buttonSensorReading *buttonSensoractiveReading =
		(struct buttonSensorReading *)buttonSensor.activeReading;

	if (buttonSensor.status == SENSOR_OK)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"button\":\"%d\"",
				 jsonBuffer,
				 buttonSensoractiveReading->pressed);
	}
}

void buttonSensorStatusMessage(char *buffer, int bufferLength)
{

	if (buttonSensorSettings.buttonSensorFitted)
	{
		struct buttonSensorReading *buttonSensoractiveReading =
			(struct buttonSensorReading *)buttonSensor.activeReading;

		if (buttonSensoractiveReading->pressed)
			snprintf(buffer, bufferLength, "Button pressed");
		else
			snprintf(buffer, bufferLength, "Button released");
	}
	else
	{
		snprintf(buffer, bufferLength, "Button sensor not fitted");
	}
}

struct sensor buttonSensor = {
	"button",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startbuttonSensor,
	stopButtonSensor,
	updateButtonSensorReading,
	startButtonSensorReading,
	addButtonSensorReading,
	buttonSensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&buttonSensorSettings,
	sizeof(struct ButtonSensorSettings),
	&buttonSensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	ButtonSensorListenerFunctions,
	sizeof(ButtonSensorListenerFunctions) / sizeof(struct sensorEventBinder)};
