#include "sensors.h"
#include "pixels.h"
#include "settings.h"
#include "controller.h"

struct sensor *activeSensorList = NULL;
struct sensor *allSensorList = NULL;

void addSensorToAllSensorsList(struct sensor *newSensor)
{
	newSensor->nextAllSensors = NULL;

	if (allSensorList == NULL)
	{
		allSensorList = newSensor;
	}
	else
	{
		sensor *addPos = allSensorList;

		while (addPos->nextAllSensors != NULL)
		{
			addPos = addPos->nextAllSensors;
		}
		addPos->nextAllSensors = newSensor;
	}
}

void addMessageListenerToSensor(struct sensor *sensor, struct sensorListener *listener)
{
	listener->nextMessageListener = NULL;

	if (sensor->listeners == NULL)
	{
		sensor->listeners = listener;
	}
	else
	{
		sensorListener *addPos = sensor->listeners;

		while (addPos->nextMessageListener != NULL)
		{
			addPos = addPos->nextMessageListener;
		}
		addPos->nextMessageListener = listener;
	}
}

void addSensorToActiveSensorsList(struct sensor *newSensor)
{
	newSensor->nextActiveSensor = NULL;

	if (activeSensorList == NULL)
	{
		activeSensorList = newSensor;
	}
	else
	{
		sensor *addPos = activeSensorList;

		while (addPos->nextActiveSensor != NULL)
		{
			addPos = addPos->nextActiveSensor;
		}
		addPos->nextActiveSensor = newSensor;
	}
}

struct sensor *findSensorByName(const char *name)
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (strcasecmp(allSensorPtr->sensorName, name) == 0)
		{
			return allSensorPtr;
		}
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
	return NULL;
}

struct sensor *findSensorSettingCollectionByName(const char *name)
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (allSensorPtr->settingItems == NULL)
			continue;

		if (strcasecmp(allSensorPtr->settingItems->collectionName, name) == 0)
		{
			return allSensorPtr;
		}
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
	return NULL;
}

#define SENSOR_STATUS_BUFFER_SIZE 300

char sensorStatusBuffer[SENSOR_STATUS_BUFFER_SIZE];

#define SENSOR_VALUE_BUFFER_SIZE 300

char sensorValueBuffer[SENSOR_VALUE_BUFFER_SIZE];

void startSensors()
{
	Serial.println("Starting sensors");
	// start all the sensor managers

	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		Serial.printf("   %s: ", activeSensorPtr->sensorName);
		activeSensorPtr->startSensor();
		activeSensorPtr->getStatusMessage(sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		Serial.printf("%s\n", sensorStatusBuffer);
		activeSensorPtr->beingUpdated = true;
		addStatusItem(activeSensorPtr->status == SENSOR_OK);
		renderStatusDisplay();
		activeSensorPtr = activeSensorPtr->nextAllSensors;
	}
}

void dumpSensorStatus()
{
	Serial.println("Sensors");
	unsigned long currentMillis = millis();

	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		activeSensorPtr->getStatusMessage(sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		sensorValueBuffer[0] = 0; // empty the buffer string
		activeSensorPtr->addReading(sensorValueBuffer, SENSOR_VALUE_BUFFER_SIZE);
		Serial.printf("    %s  %s Active time(microsecs): ",
					  sensorStatusBuffer, sensorValueBuffer);
		Serial.print(activeSensorPtr->activeTime);
		Serial.print("  Millis since last reading: ");
		Serial.println(ulongDiff(currentMillis, activeSensorPtr->millisAtLastReading));

		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}
}

void startSensorsReading()
{
	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		if (activeSensorPtr->beingUpdated)
		{
			activeSensorPtr->startReading();
		}
		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}
}

void updateSensors()
{
	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		if (activeSensorPtr->beingUpdated)
		{
			unsigned long startMicros = micros();
			activeSensorPtr->updateSensor();
			activeSensorPtr->activeTime = ulongDiff(micros(), startMicros);
		}
		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}
}

void createSensorJson(char *deviceName, char *buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "{ \"dev\":\"%s\"", deviceName);

	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		if (activeSensorPtr->beingUpdated)
		{
			activeSensorPtr->addReading(buffer, bufferLength);
		}
		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}

	snprintf(buffer, bufferLength, "%s}", buffer);
}

void displaySensorStatus()
{
	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		addStatusItem(activeSensorPtr->status == SENSOR_OK);
		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}
}

void stopSensors()
{
	Serial.println("Stopping sensors");

	sensor *activeSensorPtr = activeSensorList;

	while (activeSensorPtr != NULL)
	{
		Serial.printf("   %s\n", activeSensorPtr->sensorName);
		activeSensorPtr->stopSensor();
		activeSensorPtr = activeSensorPtr->nextActiveSensor;
	}
}

void iterateThroughSensors(void (*func)(sensor *s))
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		func(allSensorPtr);
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
}

void iterateThroughSensorSettingCollections(void (*func)(SettingItemCollection *s))
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (allSensorPtr->settingItems != NULL)
		{
			func(allSensorPtr->settingItems);
		}
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
}

void iterateThroughSensorSettings(void (*func)(unsigned char *settings, int size))
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		//Serial.printf("  Settings %s\n", allSensorPtr->sensorName);
		func(allSensorPtr->settingsStoreBase,
			 allSensorPtr->settingsStoreLength);
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
}

void iterateThroughSensorSettings(void (*func)(SettingItem *s))
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (allSensorPtr->settingItems != NULL)
		{
			for (int j = 0; j < allSensorPtr->settingItems->noOfSettings; j++)
			{
				func(allSensorPtr->settingItems->settings[j]);
			}
		}
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
}

void resetSensorsToDefaultSettings()
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (allSensorPtr->settingItems != NULL)
		{
			for (int j = 0; j < allSensorPtr->settingItems->noOfSettings; j++)
			{
				void *dest = allSensorPtr->settingItems->settings[j]->value;
				allSensorPtr->settingItems->settings[j]->setDefault(dest);
			}
		}
		allSensorPtr = allSensorPtr->nextAllSensors;
	}
}

SettingItem *FindSensorSettingByFormName(const char *settingName)
{
	sensor *allSensorPtr = allSensorList;

	while (allSensorPtr != NULL)
	{
		if (allSensorPtr->settingItems != NULL)
		{
			SettingItemCollection *testItems = allSensorPtr->settingItems;

			for (int j = 0; j < allSensorPtr->settingItems->noOfSettings; j++)
			{
				SettingItem *testSetting = testItems->settings[j];
				if (matchSettingName(testSetting, settingName))
				{
					return testSetting;
				}
			}
			allSensorPtr = allSensorPtr->nextAllSensors;
		}
	}
	return NULL;
}

struct sensorEventBinder *findSensorListenerByName(struct sensor *s, const char *name)
{
	if (s->sensorListenerFunctions == NULL)
	{
		return NULL;
	}

	if (s->noOfSensorListenerFunctions == 0)
	{
		return NULL;
	}

	for (int i = 0; i < s->noOfSensorListenerFunctions; i++)
	{
		sensorEventBinder *binder = &s->sensorListenerFunctions[i];
		if (strcasecmp(binder->listenerName, name) == 0)
		{
			return binder;
		}
	}
	return NULL;
}

void iterateThroughSensorListeners(struct sensor *sensor, void (*func)(struct sensorListner *listener))
{
	struct sensorListener *pos = sensor->listeners;

	while (pos != NULL)
	{
		func((sensorListner *)pos);
		pos = pos->nextMessageListener;
	}
}
 
void fireSensorListenersOnMaskBit(struct sensor *sensor, int mask)
{
	struct sensorListener *pos = sensor->listeners;

	//Serial.printf("      Sensor:%s mask:%d\n", sensor->sensorName, mask);

	while (pos != NULL)
	{
		//Serial.printf("        Listener:%s sendoption:%d\n", pos->config->listenerName, pos->config->sendOptionMask);
		if ((pos->config->sendOptionMask & mask) != 0)
		{
			//Serial.println("Got a match");
			// dumpCommand(pos->config->commandProcess, pos->config->commandName, pos->config->optionBuffer);

			pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
			//Serial.println("command performed");
		}
		pos = pos->nextMessageListener;
	}
}
