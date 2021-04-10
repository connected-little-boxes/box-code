#include "potSensor.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"
#include "mqtt.h"
#include "controller.h"
#include "pixels.h"
#include "buttonsensor.h"

struct PotSensorSettings potSensorSettings;

void setDefaultPotInputPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

void setDefaultPotMillisBetweenReadings(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 100;
}

void setDefaultPotDeadZone(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 10;
}

struct SettingItem PotSensorInputPinNoSetting = {
	"Pot sensor data Pin",
	"potsensordatapin",
	&potSensorSettings.potSensorDataPinNo,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultPotInputPinNo,
	validateInt};

struct SettingItem potSensorFittedSetting = {
	"Pot sensor fitted (yes or no)",
	"potsensorfitted",
	&potSensorSettings.potSensorFitted,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem PotSensorMillisBetweenReadings = {
	"Pot sensor millis between readings",
	"potsensormillisbetween",
	&potSensorSettings.millisBetweenReadings,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultPotMillisBetweenReadings,
	validateInt};

struct SettingItem PotSensorDeadZone = {
	"Pot sensor dead zone",
	"potsensordeadzone",
	&potSensorSettings.potDeadZone,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultPotDeadZone,
	validateInt};


struct SettingItem *potSensorSettingItemPointers[] =
	{
		&PotSensorInputPinNoSetting,
		&potSensorFittedSetting,
		&PotSensorMillisBetweenReadings,
		&PotSensorDeadZone
	};

struct SettingItemCollection potSensorSettingItems = {
	"potSensor",
	"potSensor hardware",
	potSensorSettingItemPointers,
	sizeof(potSensorSettingItemPointers) / sizeof(struct SettingItem *)};

struct sensorEventBinder POTSensorListenerFunctions[] = {
	{"turned", POTSENSOR_SEND_ON_POS_CHANGE}};

void readPOTSensor(struct potSensorReading *potSensoractiveReading)
{
	potSensoractiveReading->counter = analogRead(potSensorSettings.potSensorDataPinNo);
//	potSensoractiveReading->counter = 100;
}

void updatePOTSensor()
{
	if(WiFiProcessDescriptor.status != WIFI_OK && WiFiProcessDescriptor.status != WIFI_TURNED_OFF )
	{
		return;
	}

	unsigned long currentMillis = millis();

	unsigned long ulMillisSinceLastPotUpdate = ulongDiff(currentMillis, potSensor.millisAtLastReading);

	int millisSinceLastPotUpdate = (int) ulMillisSinceLastPotUpdate;

	if(millisSinceLastPotUpdate<potSensorSettings.millisBetweenReadings)
	{
		return;
	}

	struct potSensorReading *potSensoractiveReading =
		(struct potSensorReading *)potSensor.activeReading;

	readPOTSensor(potSensoractiveReading);

	int readingChange = potSensoractiveReading->counter - potSensoractiveReading->previousPotReading;

	if(readingChange < 0)
	{
		readingChange = readingChange * -1;
	}

	if(readingChange < potSensorSettings.potDeadZone)
	{
		return ;
	}

	potSensoractiveReading->previousPotReading = potSensoractiveReading->counter;

	potSensor.millisAtLastReading = currentMillis;

	// work through the listeners and post messages where requested

	sensorListener *pos = potSensor.listeners;

	while (pos != NULL)
	{
		if (pos->config->sendOptionMask & POTSENSOR_SEND_ON_POS_CHANGE)
		{
			// if the command has a value element we now need to take the element value and put
			// it into the command data for the message that is about to be received.
			// The command data value is always the first item in the parameter block

			float resultValue = (float)potSensoractiveReading->counter/1024;
			putUnalignedFloat(resultValue, (unsigned char *) &pos->config->optionBuffer);
			pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
			pos->lastReadingMillis = buttonSensor.millisAtLastReading;
			// move on to the next one
			pos = pos->nextMessageListener;
			continue;
		}

		// move on to the next one
		pos = pos->nextMessageListener;
	}
}

void potSensorTest()
{
	struct potSensorReading *potSensoractiveReading =
		(struct potSensorReading *)potSensor.activeReading;

	Serial.println("Pot Sensor test\nPress the ESC key to end the test");

	while (true)
	{
		if (Serial.available() != 0)
		{
			int ch = Serial.read();
			if (ch == ESC_KEY)
			{
				break;
			}
		}


		readPOTSensor(potSensoractiveReading);

		Serial.printf("Pot value:%d\n", potSensoractiveReading->counter);

		delay(100);
	}

	Serial.println("Pot test ended");
}

void startPotSensor()
{
	if (potSensor.activeReading == NULL)
	{
		potSensor.activeReading = new potSensorReading();
	}

	struct potSensorReading *potSensoractiveReading =
		(struct potSensorReading *)potSensor.activeReading;

	if (!potSensorSettings.potSensorFitted)
	{
		potSensor.status = POTSENSOR_NOT_FITTED;
	}
	else
	{
		potSensoractiveReading->previousPotReading = -potSensorSettings.potDeadZone;
		potSensor.millisAtLastReading = millis() - potSensorSettings.millisBetweenReadings;
		potSensor.status = SENSOR_OK;
	}
}

void stopPotSensor()
{
}

void updatePotSensorReading()
{
	switch (potSensor.status)
	{
	case SENSOR_OK:
		updatePOTSensor();
		break;

	case POTSENSOR_NOT_FITTED:
		break;
	}
}

void startPotSensorReading()
{
}

void addPotSensorReading(char *jsonBuffer, int jsonBufferSize)
{
	struct potSensorReading *potSensoractiveReading =
		(struct potSensorReading *)potSensor.activeReading;

	if (potSensor.status == SENSOR_OK)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"pot\":\"%d\"",
				 jsonBuffer,
				 potSensoractiveReading->counter);
	}
}

void potSensorStatusMessage(char *buffer, int bufferLength)
{
	struct potSensorReading *potSensoractiveReading =
		(struct potSensorReading *)potSensor.activeReading;

	switch (potSensor.status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "Reading:%d", potSensoractiveReading->counter);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "Pot sensor off");
		break;

	case POTSENSOR_NOT_FITTED:
		snprintf(buffer, bufferLength, "Pot sensor not fitted");
		break;

	default:
		snprintf(buffer, bufferLength, "Pot sensor status invalid");
		break;
	}
}

struct sensor potSensor = {
	"pot",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startPotSensor,
	stopPotSensor,
	updatePotSensorReading,
	startPotSensorReading,
	addPotSensorReading,
	potSensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&potSensorSettings,
	sizeof(struct PotSensorSettings),
	&potSensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	POTSensorListenerFunctions,
	sizeof(POTSensorListenerFunctions) / sizeof(struct sensorEventBinder)};
