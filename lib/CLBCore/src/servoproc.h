#pragma once

#define SERVO_OK 900
#define SERVO_STOPPED 901
#define SERVO_COMMAND_NAME_LENGTH 40
#define SERVO_MAX_HOLD_TIME_SECS 10

// These override the Min and Max values in servo code which are 
// set at 1000 and 2000 in the latest version of the Servo libraries
// These were the values in earlier versions 

// See the discussions here:
// https://github.com/esp8266/Arduino/issues/8081
// https://github.com/esp8266/Arduino/pull/7023


#define SERVO_MIN 544
#define SERVO_MAX 2400



struct ServoSettings {
	int ServoOutputPin;
	float ServoInitialPosition;
	bool ServoEnabled;
};

int setServoPosition(float position);

int startServo(struct process * servoProcess);

int updateServo(struct process * servoProcess);

int stopServo(struct process * servoProcess);

void ServoStatusMessage(struct process * servoProcess, char * buffer, int bufferLength);

extern struct ServoSettings ServoSettings;

extern struct SettingItemCollection ServoSettingItems;

struct ServoCommandItems
{
	int position;
};

extern struct ServoCommandItems servoCommandItems;

extern struct process ServoProcess;
