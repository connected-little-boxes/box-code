#pragma once

#include <Arduino.h>
#include "settings.h"

#define ROTARY_READING_LIFETIME_MSECS 5000

#define ROTARYSENSOR_NOT_FITTED -1
#define ROTARYSENSOR_NOT_CONNECTED -2

#define ROTARYSENSOR_SEND_ON_COUNT_CHANGE_MASK_BIT 1
#define ROTARYSENSOR_SEND_ON_PRESSED_MASK_BIT 2
#define ROTARYSENSOR_SEND_ON_RELEASED_MASK_BIT 4

struct rotarySensorReading {
	bool pressed;
	int counter;
	bool direction;
};

struct RotarySensorSettings {
	int rotarySensorDataPinNo;
	int rotarySensorClockPinNo;
	int rotarySensorSwitchPinNo;
	bool rotarySensorFitted;
};

extern struct RotarySensorSettings rotarySensorSettings;

extern struct SettingItemCollection rotarySensorSettingItems;

int startrotarySensor(struct sensor * rotarySensorSensor);
int stoprotarySensor(struct sensor* rotarySensorSensor);
int updaterotarySensorReading(struct sensor * rotarySensorSensor);
void startrotarySensorReading(struct sensor * rotarySensorSensor);
int addrotarySensorReading(struct sensor * rotarySensorSensor, char * jsonBuffer, int jsonBufferSize);
void rotarySensorStatusMessage(struct sensor * rotarySensorsensor, char * buffer, int bufferLength);
void rotarySensorTest();

extern struct sensor rotarySensor;
