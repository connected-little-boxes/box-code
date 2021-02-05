#pragma once

#define SERVO_OK 900
#define SERVO_STOPPED 901
#define SERVO_COMMAND_NAME_LENGTH 40

struct ServoSettings {
	int ServoOutputPin;
	double ServoInitialPosition;
	bool ServoEnabled;
};

int setServoPosition(float position);

int startServo(struct process * ServoProcess);

int updateServo(struct process * ServoProcess);

int stopServo(struct process * ServoProcess);

void ServoStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct ServoSettings ServoSettings;

extern struct SettingItemCollection ServoSettingItems;

struct ServoCommandItems
{
	int position;
};

extern struct ServoCommandItems servoCommandItems;

extern struct process ServoProcess;
