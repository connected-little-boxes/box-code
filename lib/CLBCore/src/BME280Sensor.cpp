#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Math.h>

#include "BME280Sensor.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"
#include "mqtt.h"
#include "controller.h"
#include "pixels.h"
#include "clock.h"

struct BME280SensorSettings bme280SensorSettings;

void setDefaultEnvnoOfAverages(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 25;
}

struct SettingItem bme280SensorFittedSetting = {
	"BME 280 sensor fitted (yes or no)",
	"bme280sensorfitted",
	&bme280SensorSettings.bme280SensorFitted,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem envNoOfAveragesSetting = {
	"Environment Number of averages",
	"envnoOfAverages",
	&bme280SensorSettings.envNoOfAverages,
	NUMBER_INPUT_LENGTH,
	integerValue,
	setDefaultEnvnoOfAverages,
	validateInt};

void setDefaultEnvTempDelta(void *dest)
{
	float *destFloat = (float *)dest;
	*destFloat = 0.5;
}

struct SettingItem BME280TempDelta = {
	"Temperature change to trigger transmit",
	"tempchangetoxmit",
	&bme280SensorSettings.tempDelta,
	NUMBER_INPUT_LENGTH,
	floatValue,
	setDefaultEnvTempDelta,
	validateFloat};

void setDefaultEnvPressDelta(void *dest)
{
	float *destFloat = (float *)dest;
	*destFloat = 10;
}

struct SettingItem BME280PressDelta = {
	"Pressure change to trigger transmit",
	"presschangetoxmit",
	&bme280SensorSettings.pressDelta,
	NUMBER_INPUT_LENGTH,
	floatValue,
	setDefaultEnvPressDelta,
	validateFloat};

void setDefaultEnvHumidDelta(void *dest)
{
	float *destFloat = (float *)dest;
	*destFloat = 2;
}

struct SettingItem BME280HumidDelta = {
	"Humidity change to trigger transmit",
	"humidchangetoxmit",
	&bme280SensorSettings.humidDelta,
	NUMBER_INPUT_LENGTH,
	floatValue,
	setDefaultEnvHumidDelta,
	validateFloat};

struct SettingItem *bme280SensorSettingItemPointers[] =
	{
		&bme280SensorFittedSetting,
		&BME280TempDelta,
		&BME280PressDelta,
		&BME280HumidDelta,
		&envNoOfAveragesSetting,
};

struct SettingItemCollection bme280SensorSettingItems = {
	"bme280Sensor",
	"bme280Sensor hardware",
	bme280SensorSettingItemPointers,
	sizeof(bme280SensorSettingItemPointers) / sizeof(struct SettingItem *)};

struct sensorEventBinder BME280SensorListenerFunctions[] = {
	{"humidsec", BME280SENSOR_SEND_HUMID_ON_SECOND},
	{"humidmin", BME280SENSOR_SEND_HUMID_ON_MINUTE},
	{"humid5min", BME280SENSOR_SEND_HUMID_ON_FIVE_MINUTE},
	{"humid30min", BME280SENSOR_SEND_HUMID_ON_HALF_HOUR},
	{"humidhour", BME280SENSOR_SEND_HUMID_ON_HOUR},
	{"humidchange", BME280SENSOR_SEND_HUMID_ON_CHANGE},

	{"presssec", BME280SENSOR_SEND_PRESS_ON_SECOND},
	{"pressmin", BME280SENSOR_SEND_PRESS_ON_MINUTE},
	{"press5min", BME280SENSOR_SEND_PRESS_ON_FIVE_MINUTE},
	{"press30min", BME280SENSOR_SEND_PRESS_ON_HALF_HOUR},
	{"presshour", BME280SENSOR_SEND_PRESS_ON_HOUR},
	{"presschange", BME280SENSOR_SEND_PRESS_ON_CHANGE},

	{"tempsec", BME280SENSOR_SEND_TEMP_ON_SECOND},
	{"tempmin", BME280SENSOR_SEND_TEMP_ON_MINUTE},
	{"temp5min", BME280SENSOR_SEND_TEMP_ON_FIVE_MINUTE},
	{"temp30min", BME280SENSOR_SEND_TEMP_ON_HALF_HOUR},
	{"temphour", BME280SENSOR_SEND_TEMP_ON_HOUR},
	{"tempchange", BME280SENSOR_SEND_TEMP_ON_CHANGE}};

Adafruit_BME280 bme;

int bmeAddresses[] = {0x76, 0x77};

void resetEnvqAverages(BME280SensorReading *reading)
{
	reading->temperatureTotal = 0;
	reading->pressureTotal = 0;
	reading->humidityTotal = 0;
	reading->averageCount = 0;
}

void updateEnvAverages(BME280SensorReading *reading)
{
	reading->temperatureTotal += reading->temperature;
	reading->pressureTotal += reading->pressure;
	reading->humidityTotal += reading->humidity;

	reading->averageCount++;

	if (reading->averageCount == bme280SensorSettings.envNoOfAverages)
	{
		reading->temperatureAverage = reading->temperatureTotal / bme280SensorSettings.envNoOfAverages;
		reading->pressureAverage = reading->pressureTotal / bme280SensorSettings.envNoOfAverages;
		reading->humidityAverage = reading->humidityTotal / bme280SensorSettings.envNoOfAverages;
		reading->lastEnvqAverageMillis = millis();
		reading->envNoOfAveragesCalculated++;
		resetEnvqAverages(reading);
	}
}

void sendBME280Humidity(BME280SensorReading *reading, sensorListener *pos)
{
	TRACELN("    BME280 humidity sent");

	unsigned char *optionBuffer = pos->config->optionBuffer;
	char *messageBuffer = (char *)optionBuffer + MESSAGE_START_POSITION;
	snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%f", reading->humidityAverage);
	float humidityNormalised = reading->humidityAverage / 100;
	putUnalignedFloat(humidityNormalised, (unsigned char *)optionBuffer);
	pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
}

void sendBME280Temp(BME280SensorReading *reading, sensorListener *pos)
{
	TRACELN("    BME280 temp sent");

	unsigned char *optionBuffer = pos->config->optionBuffer;
	char *messageBuffer = (char *)optionBuffer + MESSAGE_START_POSITION;
	snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%.1f", reading->temperatureAverage);
	float tempNormalised = reading->temperatureAverage + 20 / 60;
	putUnalignedFloat(tempNormalised, (unsigned char *)optionBuffer);
	pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
}

void sendBME280Press(BME280SensorReading *reading, sensorListener *pos)
{
	TRACELN("    BME280 pressure sent");

	unsigned char *optionBuffer = pos->config->optionBuffer;
	char *messageBuffer = (char *)optionBuffer + MESSAGE_START_POSITION;
	snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%f", reading->pressureAverage);
	float pressNormalised = reading->pressureAverage / 1500;
	putUnalignedFloat(pressNormalised, (unsigned char *)optionBuffer);
	pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
}

void sendBME280Reading(BME280SensorReading *reading, int sensorNo, sensorListener *pos)
{
	TRACE("sendBME280Reading for sensor no:");
	TRACELN(sensorNo);

	if(sensorNo&BME280_HUMID)
		sendBME280Humidity(reading, pos);

	if(sensorNo&BME280_TEMP)
		sendBME280Temp(reading, pos);

	if(sensorNo&BME280_PRESS)
		sendBME280Press(reading, pos);
}

void sendToBME280SensorListeners(int event,  int sensorNo)
{
	TRACE("sendToBME280SensorListeners event:");
	TRACE_HEX(event);
	TRACE(" sensorNo:");
	TRACE_HEXLN(sensorNo);
	struct BME280SensorReading *bme280activeReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;

	sensorListener *pos = bme280Sensor.listeners;

	while (pos != NULL)
	{
		struct sensorListenerConfiguration *config = pos->config;
		int configMask = config->sendOptionMask;

		if((configMask & BME280_EVENT_MASK) == event)
		{
			// we have a match for the event
			int configSensorNo = configMask & BME280_SENSOR_MASK;

			if (configSensorNo == sensorNo){
				sendBME280Reading(bme280activeReading, configSensorNo, pos);
			}
		}
		pos = pos->nextMessageListener;
	}
}

void sendToBME280SensorListeners(int event)
{
	TRACE("Sending BME20 listener to event:");
	TRACE_HEXLN(event);

	struct BME280SensorReading *bme280activeReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;

	sensorListener *pos = bme280Sensor.listeners;

	while (pos != NULL)
	{
		struct sensorListenerConfiguration *config = pos->config;
		int configMask = config->sendOptionMask;
		TRACE(" checking listener with ");
		TRACE_HEXLN(configMask);

		if((configMask & BME280_EVENT_MASK) == event)
		{
			TRACELN("    match");
			// we have a match for the event
			int configSensorNo = configMask & BME280_SENSOR_MASK;
			sendBME280Reading(bme280activeReading, configSensorNo, pos);
		}
		pos = pos->nextMessageListener;
	}
}

void updateBME280Sensor()
{
	static int lastClockSecond = -1;
	static int lastClockMinute = -1;
	static int lastClockHour = -1;

	struct clockReading *clockReading = (struct clockReading *)clockSensor.activeReading;

	struct BME280SensorReading *bme280activeReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;

	float temp = bme.readTemperature();

	if (isnan(temp))
	{
		bme280Sensor.status = BME280SENSOR_NOT_CONNECTED;
		return;
	}
	else
	{
		bme280activeReading->temperature = temp;       
		bme280activeReading->humidity = bme.readHumidity();
		bme280activeReading->pressure = bme.readPressure() / 100.0F;
		bme280Sensor.millisAtLastReading = millis();
		bme280Sensor.readingNumber++;
		updateEnvAverages(bme280activeReading);
	}

	bme280Sensor.millisAtLastReading = millis();

	if (fabsf(bme280activeReading->lastHumidSent - bme280activeReading->humidityAverage) > bme280SensorSettings.humidDelta)
	{
		TRACELN("Humidity change:");
		sendToBME280SensorListeners(BME280_ON_CHANGE, BME280_HUMID);
		bme280activeReading->lastHumidSent = bme280activeReading->humidityAverage;
	}

	if (fabsf(bme280activeReading->lastTempSent - bme280activeReading->temperatureAverage) > bme280SensorSettings.tempDelta)
	{
		TRACELN("Temp change:");
		sendToBME280SensorListeners(BME280_ON_CHANGE, BME280_TEMP);
		bme280activeReading->lastTempSent = bme280activeReading->temperatureAverage;
	}

	if (fabsf(bme280activeReading->lastPressSent - bme280activeReading->pressureAverage) > bme280SensorSettings.pressDelta)
	{
		TRACELN("Press change:");
		sendToBME280SensorListeners(BME280_ON_CHANGE, BME280_PRESS);
		bme280activeReading->lastPressSent = bme280activeReading->pressureAverage;
	}

	if(lastClockSecond == clockReading->second)
	{
		return;
	}

	lastClockSecond = clockReading->second;

	TRACELN("BME280 sending on second");

	sendToBME280SensorListeners(BME280_ON_SECOND);

	if(lastClockMinute == clockReading->minute)
	{
		return;
	}

	lastClockMinute = clockReading->minute;

	TRACELN("BME280 sending on minute");

	sendToBME280SensorListeners(BME280_ON_MIN);

	if(clockReading->minute % 5 == 0)
	{
		TRACELN("BME280 sending on five minutes");
		sendToBME280SensorListeners(BME280_ON_FIVE_MIN);
	}

	if(clockReading->minute % 30 == 0)
	{
		TRACELN("BME280 sending on half hour");
		sendToBME280SensorListeners(BME280_ON_HALF_HOUR);
	}

	if(lastClockHour == clockReading->hour)
	{
		return;
	}

	lastClockHour = clockReading->hour;

	TRACELN("BME280 sending half hour");
	sendToBME280SensorListeners(BME280_ON_HOUR);
}


void startBME280Sensor()
{
	if (!bme280SensorSettings.bme280SensorFitted)
	{
		bme280Sensor.status = BME280SENSOR_NOT_FITTED;
		return;
	}

	if (bme280Sensor.activeReading == NULL)
	{
		bme280Sensor.activeReading = new BME280SensorReading();
	}

	struct BME280SensorReading *bme280activeReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;

	for (int i = 0; i < sizeof(bmeAddresses) / sizeof(int); i++)
	{
		if (bme.begin(bmeAddresses[i]))
		{
			float testTemp = bme.readTemperature();

			if (isnan(testTemp))
			{
				// ignpore non-numbers
				continue;
			}

			bme280activeReading->activeBMEAddress = bmeAddresses[i];

			bme280Sensor.status = SENSOR_OK;
			return;
		}
	}

	bme280Sensor.status = BME280SENSOR_NOT_CONNECTED;
}

void stopBME280Sensor()
{
}

void updateBME280SensorReading()
{
	switch (bme280Sensor.status)
	{
	case SENSOR_OK:
		updateBME280Sensor();
		break;

	case BME280SENSOR_NOT_FITTED:
		break;

	case BME280SENSOR_NOT_CONNECTED:
		break;
	}
}

void startBME280SensorReading()
{
	struct BME280SensorReading *bme280activeReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;
	resetEnvqAverages(bme280activeReading);
}

void addBME280SensorReading(char *jsonBuffer, int jsonBufferSize)
{
	if (bme280Sensor.status == SENSOR_OK)
	{
		struct BME280SensorReading *bme280SensoractiveReading =
			(struct BME280SensorReading *)bme280Sensor.activeReading;

		if (ulongDiff(millis(), bme280SensoractiveReading->lastEnvqAverageMillis) < ENV_READING_LIFETIME_MSECS)
		{
			snprintf(jsonBuffer, jsonBufferSize, "%s,\"temp\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f",
					 jsonBuffer,
					 bme280SensoractiveReading->temperatureAverage,
					 bme280SensoractiveReading->humidityAverage,
					 bme280SensoractiveReading->pressureAverage);
		}
	}
}

void bme280SensorStatusMessage(char *buffer, int bufferLength)
{
	struct BME280SensorReading *bme280SensoractiveReading =
		(struct BME280SensorReading *)bme280Sensor.activeReading;

	switch (bme280Sensor.status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "Temp:%.2f Humidity:%.2f pressure:%.2f",
				 bme280SensoractiveReading->temperatureAverage,
				 bme280SensoractiveReading->humidityAverage,
				 bme280SensoractiveReading->pressureAverage);

		break;

	case BME280SENSOR_NOT_CONNECTED:
		snprintf(buffer, bufferLength, "BME280 not connected");
		break;

	case BME280SENSOR_NOT_FITTED:
		snprintf(buffer, bufferLength, "BME280 not fitted");
		break;

	default:
		snprintf(buffer, bufferLength, "PIR sensor status invalid");
		break;
	}
}

struct sensor bme280Sensor = {
	"BME280",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startBME280Sensor,
	stopBME280Sensor,
	updateBME280SensorReading,
	startBME280SensorReading,
	addBME280SensorReading,
	bme280SensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&bme280SensorSettings,
	sizeof(struct BME280SensorSettings),
	&bme280SensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	BME280SensorListenerFunctions,
	sizeof(BME280SensorListenerFunctions) / sizeof(struct sensorEventBinder)};
