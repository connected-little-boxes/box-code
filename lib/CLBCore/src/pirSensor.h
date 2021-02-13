#pragma once

#include <Arduino.h>
#include "settings.h"

#define PIR_READING_LIFETIME_MSECS 5000

#define PIRSENSOR_NOT_FITTED -1
#define PIRSENSOR_NOT_CONNECTED -2

#define PIRSENSOR_SEND_ON_CHANGE 1
#define PIRSENSOR_SEND_ON_TRIGGERED 2
#define PIRSENSOR_SEND_ON_CLEAR 4

struct pirSensorReading {
	bool triggered;
};

struct PirSensorSettings {
	int pirSensorPinNo;
	bool pirSensorFitted;
	bool pirSensorInputPinActiveHigh;
};

extern struct PirSensorSettings pirSensorSettings;

extern struct SettingItemCollection pirSensorSettingItems;

int startpirSensor(struct sensor * pirSensorSensor);
int stoppirSensor(struct sensor* pirSensorSensor);
int updatepirSensorReading(struct sensor * pirSensorSensor);
void startpirSensorReading(struct sensor * pirSensorSensor);
int addpirSensorReading(struct sensor * pirSensorSensor, char * jsonBuffer, int jsonBufferSize);
void pirSensorStatusMessage(struct sensor * pirSensorsensor, char * buffer, int bufferLength);
void pirSensorTest();

extern struct sensor pirSensor;
