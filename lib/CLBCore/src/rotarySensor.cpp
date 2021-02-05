#include "rotarySensor.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"
#include "mqtt.h"
#include "controller.h"
#include "pixels.h"
#include "buttonsensor.h"

struct RotarySensorSettings rotarySensorSettings;

void setDefaultRotaryInputPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 4;
}

void setDefaultRotaryClockPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 5;
}

void setDefaultRotarySwitchPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem RotarySensorInputPinNoSetting = {
	"Rotary sensor data Pin",
	"rotarysensordatapin",
	&rotarySensorSettings.rotarySensorDataPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultRotaryInputPinNo,
	validateInt};

struct SettingItem RotarySensorClockPinNoSetting = {
	"Rotary sensor clock Pin",
	"rotarysensorclockpin",
	&rotarySensorSettings.rotarySensorClockPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultRotaryClockPinNo,
	validateInt};

struct SettingItem ROTARYSensorPinNoSetting = {
	"Rotary sensor switch pin",
	"rotarysensorswitchpin",
	&rotarySensorSettings.rotarySensorSwitchPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultRotarySwitchPinNo,
	validateInt};

struct SettingItem rotarySensorFittedSetting = {
	"Rotary sensor fitted (yes or no)",
	"rotarysensorfitted",
	&rotarySensorSettings.rotarySensorFitted,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem *rotarySensorSettingItemPointers[] =
	{
		&RotarySensorInputPinNoSetting,
		&RotarySensorClockPinNoSetting,
		&ROTARYSensorPinNoSetting,
		&rotarySensorFittedSetting};

struct SettingItemCollection rotarySensorSettingItems = {
	"rotarySensor",
	"rotarySensor hardware",
	rotarySensorSettingItemPointers,
	sizeof(rotarySensorSettingItemPointers) / sizeof(struct SettingItem *)};

struct sensorEventBinder ROTARYSensorListenerFunctions[] = {
	{"turned", ROTARYSENSOR_SEND_ON_COUNT_CHANGE_MASK_BIT},
	{"pressed", ROTARYSENSOR_SEND_ON_PRESSED_MASK_BIT},
	{"released", ROTARYSENSOR_SEND_ON_RELEASED_MASK_BIT}};

volatile int counter = 0;
int oldCounter = 0;
volatile int currentStateCLK;
volatile int lastStateCLK;
volatile bool forward;

int lastRotaryButtonInputValue;
long rotaryButtonInputDebounceStartTime;
unsigned long millisAtLastRotaryButtonInputChange;

void readROTARYSensor(struct rotarySensorReading *rotarySensoractiveReading)
{
	rotarySensoractiveReading->counter = counter;
	rotarySensoractiveReading->direction = forward;

	int newInputValue = digitalRead(rotarySensorSettings.rotarySensorSwitchPinNo);

	if (newInputValue == lastRotaryButtonInputValue)
	{
		rotaryButtonInputDebounceStartTime = millis();
	}
	else
	{
		long currentMillis = millis();
		long millisSinceChange = ulongDiff(currentMillis, rotaryButtonInputDebounceStartTime);

		if (++millisSinceChange > BUTTON_INPUT_DEBOUNCE_TIME)
		{
			if (newInputValue)
			{
				rotarySensoractiveReading->pressed = false;
			}
			else
			{
				rotarySensoractiveReading->pressed = true;
			}
			lastRotaryButtonInputValue = newInputValue;
		}
	}
}

#define MIN_PULSE_WIDTH_MILLIS 1

void ICACHE_RAM_ATTR clockChange()
{
	// Fired when the clock input changes state
	// Read the current state of CLK
	static unsigned long lastPulseMillis = 0;
	unsigned long intMillis = millis();

	unsigned long timeFromLastInterrupt = intMillis - lastPulseMillis;

	if (timeFromLastInterrupt < MIN_PULSE_WIDTH_MILLIS)
	{
		return;
	}

	if (digitalRead(rotarySensorSettings.rotarySensorClockPinNo))
	{
		// clock has gone from low to high
		if (digitalRead(rotarySensorSettings.rotarySensorDataPinNo))
		{
			forward = false;
			if(counter>0)
				counter--;
		}
		else
		{
			forward = true;
			counter++;
		}
	}
	else
	{
		// clock has gone from hight to low
		if (digitalRead(rotarySensorSettings.rotarySensorDataPinNo))
		{
			forward = true;
			counter++;
		}
		else
		{
			forward = false;
			if(counter>0)
				counter--;
		}
	}

	lastPulseMillis = intMillis;
}

bool updateROTARYSensor()
{
	struct rotarySensorReading *rotarySensoractiveReading =
		(struct rotarySensorReading *)rotarySensor.activeReading;

	bool previousPressed = rotarySensoractiveReading->pressed;
	int previousCounter = rotarySensoractiveReading->counter;

	readROTARYSensor(rotarySensoractiveReading);

	rotarySensor.millisAtLastReading = millis();

	// work through the listeners and post messages where requested

	sensorListener *pos = rotarySensor.listeners;

	while (pos != NULL)
	{
		if (previousPressed != rotarySensoractiveReading->pressed)
		{

			if (pos->config->sendOptionMask & ROTARYSENSOR_SEND_ON_PRESSED_MASK_BIT)
			{
				// send on pressed - is the button pressed now?
				if (rotarySensoractiveReading->pressed)
				{
					pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
					pos->lastReadingMillis = buttonSensor.millisAtLastReading;
					// move on to the next one
					pos = pos->nextMessageListener;
					continue;
				}
			}

			if (pos->config->sendOptionMask & ROTARYSENSOR_SEND_ON_RELEASED_MASK_BIT)
			{
				// send on released - is the button released now?
				if (!rotarySensoractiveReading->pressed)
				{
					pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
					pos->lastReadingMillis = buttonSensor.millisAtLastReading;
					// move on to the next one
					pos = pos->nextMessageListener;
					continue;
				}
			}
		}

		if (pos->config->sendOptionMask & ROTARYSENSOR_SEND_ON_COUNT_CHANGE_MASK_BIT)
		{
			if (rotarySensoractiveReading->counter != previousCounter)
			{
				// if the command has a value element we now need to take the element value and put
				// it into the command data for the message that is about to be received.
				// The command data value is always the first item in the parameter block

				float resultValue = (float)rotarySensoractiveReading->counter/100.0;
				putUnalignedFloat(resultValue, (unsigned char *) &pos->config->optionBuffer);
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

void rotarySensorTest()
{
	pinMode(rotarySensorSettings.rotarySensorDataPinNo, INPUT);

	Serial.println("Rotary Sensor test\nPress the enter key to end the test");

	while (true)
	{
		if (Serial.available() != 0)
		{
			int ch = Serial.read();
			if (ch == 0x0d)
			{
				break;
			}
		}

		Serial.printf("Direction:%d Counter:%d\n", forward, counter);

		delay(100);
	}

	Serial.println("Rotary test ended");
}

void startRotarySensor()
{
	if (rotarySensor.activeReading == NULL)
	{
		rotarySensor.activeReading = new rotarySensorReading();
	}

	if (!rotarySensorSettings.rotarySensorFitted)
	{
		rotarySensor.status = ROTARYSENSOR_NOT_FITTED;
	}
	else
	{

		pinMode(rotarySensorSettings.rotarySensorDataPinNo, INPUT);
		pinMode(rotarySensorSettings.rotarySensorClockPinNo, INPUT);
		pinMode(rotarySensorSettings.rotarySensorSwitchPinNo, INPUT);

		attachInterrupt(rotarySensorSettings.rotarySensorClockPinNo, clockChange, HIGH);

		rotarySensor.status = SENSOR_OK;
	}
}

void stopRotarySensor()
{
}

void updateRotarySensorReading()
{
	switch (rotarySensor.status)
	{
	case SENSOR_OK:
		updateROTARYSensor();
		break;

	case ROTARYSENSOR_NOT_FITTED:
		break;
	}
}

void startRotarySensorReading()
{
}

void addRotarySensorReading(char *jsonBuffer, int jsonBufferSize)
{
	struct rotarySensorReading *rotarySensoractiveReading =
		(struct rotarySensorReading *)rotarySensor.activeReading;

	if (rotarySensor.status == SENSOR_OK)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"rotary\":\"%d\"",
				 jsonBuffer,
				 rotarySensoractiveReading->counter);
	}
}

void rotarySensorStatusMessage(char *buffer, int bufferLength)
{
	switch (rotarySensor.status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "Count:%d Direction:%d", counter, forward);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "Rotary sensor off");
		break;

	case ROTARYSENSOR_NOT_FITTED:
		snprintf(buffer, bufferLength, "Rotary sensor not fitted");
		break;

	default:
		snprintf(buffer, bufferLength, "Rotary sensor status invalid");
		break;
	}
}

struct sensor rotarySensor = {
	"rotary",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startRotarySensor,
	stopRotarySensor,
	updateRotarySensorReading,
	startRotarySensorReading,
	addRotarySensorReading,
	rotarySensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&rotarySensorSettings,
	sizeof(struct RotarySensorSettings),
	&rotarySensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	ROTARYSensorListenerFunctions,
	sizeof(ROTARYSensorListenerFunctions) / sizeof(struct sensorEventBinder)};
