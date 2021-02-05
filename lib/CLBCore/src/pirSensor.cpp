#include "pirSensor.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"
#include "mqtt.h"
#include "controller.h"
#include "pixels.h"

struct PirSensorSettings pirSensorSettings;

void setDefaultPIRPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 4;
}

struct SettingItem PIRSensorPinNoSetting = {
	"PIR sensor Input Pin",
	"pirsensorinputpin",
	&pirSensorSettings.pirSensorPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultPIRPinNo,
	validateInt};

struct SettingItem pirSensorFittedSetting = {
	"PIR sensor fitted (yes or no)",
	"pirsensorfitted",
	&pirSensorSettings.pirSensorFitted,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem pirSensorInputPinActiveHighSetting = {
	"PIR sensor input active high",
	"piractivehigh",
	&pirSensorSettings.pirSensorInputPinActiveHigh,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem *pirSensorSettingItemPointers[] =
	{
		&pirSensorFittedSetting,
		&PIRSensorPinNoSetting,
		&pirSensorInputPinActiveHighSetting};

struct SettingItemCollection pirSensorSettingItems = {
	"pirSensor",
	"pirSensor hardware",
	pirSensorSettingItemPointers,
	sizeof(pirSensorSettingItemPointers) / sizeof(struct SettingItem *)};

struct sensorEventBinder PIRSensorListenerFunctions[] = {
	{"change", PIRSENSOR_SEND_ON_CHANGE_MASK_BIT},
	{"triggered", PIRSENSOR_SEND_ON_TRIGGERED_MASK_BIT},
	{"clear", PIRSENSOR_SEND_ON_CLEAR_MASK_BIT}};

void readPIRSensor(struct pirSensorReading *pirSensoractiveReading)
{

	if (digitalRead(pirSensorSettings.pirSensorPinNo))
	{
		if (pirSensorSettings.pirSensorInputPinActiveHigh)
		{
			pirSensoractiveReading->triggered = true;
		}
		else
		{
			pirSensoractiveReading->triggered = false;
		}
	}
	else
	{
		if (pirSensorSettings.pirSensorInputPinActiveHigh)
		{
			pirSensoractiveReading->triggered = false;
		}
		else
		{
			pirSensoractiveReading->triggered = true;
		}
	}
}

bool updatePIRSensor()
{
	struct pirSensorReading *pirSensoractiveReading =
		(struct pirSensorReading *)pirSensor.activeReading;

	bool previousReading = pirSensoractiveReading->triggered;

	readPIRSensor(pirSensoractiveReading);

	pirSensor.millisAtLastReading = millis();

	if (pirSensoractiveReading->triggered == previousReading)
	{
		// we have read the PIR but the value has not changed
		// just return that we worked OK
		return true;
	}

	sensorListener *pos = pirSensor.listeners;

	while (pos != NULL)
	{
		struct sensorListenerConfiguration *config = pos->config;

		if (config->sendOptionMask & PIRSENSOR_SEND_ON_CHANGE_MASK_BIT)
		{
			pos->receiveMessage(config->destination, config->optionBuffer);
			pos->lastReadingMillis = pirSensor.millisAtLastReading;
			pos = pos->nextMessageListener;
			continue;
		}

		if (config->sendOptionMask & PIRSENSOR_SEND_ON_TRIGGERED_MASK_BIT)
		{
			if (pirSensoractiveReading->triggered)
			{
				pos->receiveMessage(config->destination, config->optionBuffer);
				pos->lastReadingMillis = pirSensor.millisAtLastReading;
				pos = pos->nextMessageListener;
				continue;
			}
		}

		if (config->sendOptionMask & PIRSENSOR_SEND_ON_CLEAR_MASK_BIT)
		{
			if (!pirSensoractiveReading->triggered)
			{
				pos->receiveMessage(config->destination, config->optionBuffer);
				pos->lastReadingMillis = pirSensor.millisAtLastReading;
				pos = pos->nextMessageListener;
				continue;
			}
		}
		pos = pos->nextMessageListener;
	}

	return true;
}

void pirSensorTest()
{
	struct pirSensorReading *pirSensoractiveReading =
		(struct pirSensorReading *)pirSensor.activeReading;

	pinMode(pirSensorSettings.pirSensorPinNo, INPUT);

	Serial.println("PIR Sensor test\nPress the enter key to end the test");

	int count = 0;
	bool triggered = false;

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
		readPIRSensor(pirSensoractiveReading);

		if (pirSensoractiveReading->triggered)
		{
			if (triggered == false)
			{
				count++;
				Serial.printf("    triggered: %d\n", count);
				triggered = true;
			}
		}
		else
		{
			triggered = false;
		}
		delay(100);
	}

	Serial.println("PIR test ended");
}

void startPirSensor()
{
	if (pirSensor.activeReading == NULL)
	{
		pirSensor.activeReading = new pirSensorReading();
	}

	if (!pirSensorSettings.pirSensorFitted)
	{
		pirSensor.status = PIRSENSOR_NOT_FITTED;
	}
	else
	{
		pinMode(pirSensorSettings.pirSensorPinNo, INPUT);
		pirSensor.status = SENSOR_OK;
	}
}

void stopPirSensor()
{
}

void updatePirSensorReading()
{
	switch (pirSensor.status)
	{
	case SENSOR_OK:
		updatePIRSensor();
		break;

	case PIRSENSOR_NOT_FITTED:
		break;
	}
}

void startPirSensorReading()
{
}

void addPirSensorReading(char *jsonBuffer, int jsonBufferSize)
{
	struct pirSensorReading *pirSensoractiveReading =
		(struct pirSensorReading *)pirSensor.activeReading;

	if (pirSensor.status == SENSOR_OK)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"pir\":\"%d\"",
				 jsonBuffer,
				 pirSensoractiveReading->triggered);
	}
}

void pirSensorStatusMessage(char *buffer, int bufferLength)
{
	struct pirSensorReading *pirSensoractiveReading =
		(struct pirSensorReading *)pirSensor.activeReading;

	switch (pirSensor.status)
	{
	case SENSOR_OK:
		if (pirSensoractiveReading->triggered)
		{
			snprintf(buffer, bufferLength, "PIR detecting");
		}
		else
		{
			snprintf(buffer, bufferLength, "PIR nothing");
		}

		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "PIR sensor off");
		break;

	case PIRSENSOR_NOT_FITTED:
		snprintf(buffer, bufferLength, "PIR sensor not fitted");
		break;

	default:
		snprintf(buffer, bufferLength, "PIR sensor status invalid");
		break;
	}
}

struct sensor pirSensor = {
	"PIR",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startPirSensor,
	stopPirSensor,
	updatePirSensorReading,
	startPirSensorReading,
	addPirSensorReading,
	pirSensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&pirSensorSettings,
	sizeof(struct PirSensorSettings),
	&pirSensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	PIRSensorListenerFunctions,
	sizeof(PIRSensorListenerFunctions) / sizeof(struct sensorEventBinder)};
