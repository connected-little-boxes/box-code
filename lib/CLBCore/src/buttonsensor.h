#pragma once

#include <Arduino.h>
#include "settings.h"

#define BUTTONSENSOR_NOT_FITTED -1
#define BUTTONSENSOR_STOPPED -2

#define BUTTONSENSOR_SEND_ON_CHANGE 1
#define BUTTONSENSOR_BUTTON_PRESSED 2
#define BUTTONSENSOR_BUTTON_RELEASED 3
// number of milliseconds that the signal must be stable
// before we read it
#define BUTTON_INPUT_DEBOUNCE_TIME 10


struct buttonSensorReading {
	bool pressed;
};

struct ButtonSensorSettings {
	int buttonSensorInputPinNo;
	int buttonGroundPin;
	bool buttonSensorFitted;
};

extern struct ButtonSensorSettings buttonSensorSettings;

extern struct SettingItemCollection buttonSensorSettingItems;

int startbuttonSensor(struct sensor * buttonSensorSensor);
int stopbuttonSensor(struct sensor* buttonSensorSensor);
int updatebuttonSensorReading(struct sensor * buttonSensorSensor);
void startbuttonSensorReading(struct sensor * buttonSensorSensor);
int addbuttonSensorReading(struct sensor * buttonSensorSensor, char * jsonBuffer, int jsonBufferSize);
void buttonSensorStatusMessage(struct sensor * buttonSensorsensor, char * buffer, int bufferLength);
void buttonSensorTest(struct sensor * buttonSensor);

extern struct sensor buttonSensor;
