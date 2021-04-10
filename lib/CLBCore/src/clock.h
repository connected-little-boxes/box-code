#pragma once

#include <Arduino.h>
#include <ezTime.h>

#include "utils.h"
#include "settings.h"
#include "statusled.h"
#include "processes.h"
#include "messages.h"

#define CLOCK_GOT_TIME_COMMAND_STORE "clock"

#define TIME_ZONE_NAME_LENGTH 20
#define DEFAULT_TIME_ZONE "Europe/London"

#define CLOCK_ERROR_NO_WIFI -1
#define CLOCK_ERROR_TIME_NOT_SET -2
#define CLOCK_ERROR_NEEDS_SYNC -3
#define CLOCK_STOPPED -4

#define CLOCK_SYNC_TIMEOUT 5

#define ALARM1_TRIGGER 1
#define ALARM2_TRIGGER 2
#define ALARM3_TRIGGER 3

#define TIMER1_TRIGGER 4
#define TIMER2_TRIGGER 5
#define CLOCK_SECOND_TICK 6
#define CLOCK_MINUTE_TICK 7
#define CLOCK_HOUR_TICK 8
#define CLOCK_DAY_TICK 9

struct clockReading {
	int hour;
	int minute;
	int second;
	int day;
	int month;
	int year;
	int dayOfWeek;
};

struct ClockAlarm{
	int hour;
	int minute;
	bool triggered;
	bool enabled;
	bool fireOnTimeMatch;
};

struct ClockTimer{
	int interval;
	bool enabled;
	bool singleShot;
	bool triggered;
	bool prevEnabled;
	int nextHour;
	int nextMinute;
};

struct ClockSensorSettings {
	char timeZone[TIME_ZONE_NAME_LENGTH];
	struct ClockAlarm alarm1;
	struct ClockAlarm alarm2;
	struct ClockAlarm alarm3;
	struct ClockTimer timer1;
	struct ClockTimer timer2;
};

extern struct ClockSensorSettings clockSensorSettings;

struct ClockReading {
	int hour;
	int minute;
	int second;
	int day;
	int month;
	int year;
	int dayOfWeek;
};

extern char * dayNames[] ;

extern char * monthNames[] ;

extern struct sensor clockSensor;

bool getDateAndTime(char * buffer, int bufferLength);
