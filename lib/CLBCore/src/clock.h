#pragma once

#include <Arduino.h>
#include <ezTime.h>

#include "utils.h"
#include "settings.h"
#include "statusled.h"
#include "processes.h"
#include "messages.h"

#define CLOCK_ERROR_NO_WIFI -1
#define CLOCK_ERROR_TIME_NOT_SET -2
#define CLOCK_ERROR_NEEDS_SYNC -3
#define CLOCK_STOPPED -4

#define CLOCK_SYNC_TIMEOUT 5

#define ALARM1_TRIGGER_BIT 1
#define ALARM2_TRIGGER_BIT 2
#define ALARM3_TRIGGER_BIT 4

#define TIMER1_TRIGGER_BIT 8
#define TIMER2_TRIGGER_BIT 16
#define CLOCK_SECOND_TICK 32
#define CLOCK_MINUTE_TICK 64
#define CLOCK_HOUR_TICK 128
#define CLOCK_DAY_TICK 256

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