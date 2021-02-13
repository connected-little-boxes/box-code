#pragma once

#include <Arduino.h>
#include "settings.h"

#define POT_READING_LIFETIME_MSECS 5000

#define POTSENSOR_NOT_FITTED -1
#define POTSENSOR_NOT_CONNECTED -2

#define POTSENSOR_SEND_ON_POS_CHANGE 1

struct potSensorReading {
	int counter;
	int previousPotReading;
};

struct PotSensorSettings {
	int potSensorDataPinNo;
	bool potSensorFitted;
	int millisBetweenReadings;
	int potDeadZone;
};

extern struct PotSensorSettings potSensorSettings;

extern struct SettingItemCollection potSensorSettingItems;

int startpotSensor(struct sensor * potSensorSensor);
int stoppotSensor(struct sensor* potSensorSensor);
int updatepotSensorReading(struct sensor * potSensorSensor);
void startpotSensorReading(struct sensor * potSensorSensor);
int addpotSensorReading(struct sensor * potSensorSensor, char * jsonBuffer, int jsonBufferSize);
void potSensorStatusMessage(struct sensor * potSensorsensor, char * buffer, int bufferLength);
void potSensorTest();

extern struct sensor potSensor;
