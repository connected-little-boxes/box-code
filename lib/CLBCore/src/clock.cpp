#include "clock.h"
#include "connectwifi.h"
#include "sensors.h"
#include "pixels.h"

extern Timezone homeTimezone;

char *dayNames[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

char *monthNames[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct ClockSensorSettings clockSensorSettings;

struct ClockReading clockReading;

void setDefaultTimeZone(void *dest)
{
	strcpy((char *)dest, DEFAULT_TIME_ZONE);
}

void defaultalarm1hour(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 7;
}

void defaultalarm2hour(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 12;
}

void defaultalarm3hour(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 19;
}

void defaultalarmMinute(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

void defaultalarmInterval(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 30;
}

boolean validateTimeZone(void *dest, const char *newValueStr)
{
	bool validText = validateString((char *)dest, newValueStr, TIME_ZONE_NAME_LENGTH);

	if(!validText) return false;

	validText = true;

	if (WiFiProcessDescriptor.status == WIFI_OK)
	{
		validText = homeTimezone.setLocation(newValueStr);
	}

	if(validText) strcpy((char *) dest, newValueStr);

	return validText;
}

struct SettingItem timeZoneSetting = {
	"Clock time zone", 
	"timezone", 
	clockSensorSettings.timeZone, 
	TIME_ZONE_NAME_LENGTH, 
	text, 
	setDefaultTimeZone, 
	validateTimeZone};

struct SettingItem alarm1Hour = {
	"Alarm1 hour",
	"alarm1hour",
	&clockSensorSettings.alarm1.hour,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarm1hour,
	validateInt};

struct SettingItem alarm1Min = {
	"Alarm1 min",
	"alarm1min",
	&clockSensorSettings.alarm1.minute,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarmMinute,
	validateInt};

struct SettingItem alarm1Enabled = {
	"Alarm1 enabled",
	"alarm1enabled",
	&clockSensorSettings.alarm1.enabled,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem alarm1TimeMatch = {
	"Alarm1 fire only on time match",
	"alarm1timematch",
	&clockSensorSettings.alarm1.fireOnTimeMatch,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem alarm2Hour = {
	"Alarm2 hour",
	"alarm2hour",
	&clockSensorSettings.alarm2.hour,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarm2hour,
	validateInt};

struct SettingItem alarm2Min = {
	"Alarm2 min",
	"alarm2min",
	&clockSensorSettings.alarm2.minute,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarmMinute,
	validateInt};

struct SettingItem alarm2Enabled = {
	"Alarm2 enabled",
	"alarm2enabled",
	&clockSensorSettings.alarm2.enabled,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem alarm2TimeMatch = {
	"Alarm2 fire only on time match",
	"alarm2timematch",
	&clockSensorSettings.alarm2.fireOnTimeMatch,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem alarm3Hour = {
	"Alarm3 hour",
	"alarm3hour",
	&clockSensorSettings.alarm3.hour,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarm3hour,
	validateInt};

struct SettingItem alarm3Min = {
	"Alarm3 min",
	"alarm3min",
	&clockSensorSettings.alarm3.minute,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarmMinute,
	validateInt};

struct SettingItem alarm3Enabled = {
	"Alarm3 enabled",
	"alarm3enabled",
	&clockSensorSettings.alarm3.enabled,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem alarm3TimeMatch = {
	"Alarm3 fire only on time match",
	"alarm3timematch",
	&clockSensorSettings.alarm3.fireOnTimeMatch,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem interval1 = {
	"Interval timer 1",
	"timer1",
	&clockSensorSettings.timer1.interval,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarmInterval,
	validateInt};

struct SettingItem interval1Enabled = {
	"Interval timer 1 enabled",
	"timer1enabled",
	&clockSensorSettings.timer1.enabled,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem interval1SingleShot = {
	"Interval timer 1 singleshot",
	"timer1singleshot",
	&clockSensorSettings.timer1.singleShot,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem interval2 = {
	"Interval timer 2",
	"timer2",
	&clockSensorSettings.timer2.interval,
	NUMBER_INPUT_LENGTH,
	integerValue,
	defaultalarmInterval,
	validateInt};

struct SettingItem interval2Enabled = {
	"Interval timer 2 enabled",
	"timer2enabled",
	&clockSensorSettings.timer2.enabled,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setFalse,
	validateYesNo};

struct SettingItem interval2SingleShot = {
	"Interval timer 2 singleshot",
	"timer2singleshot",
	&clockSensorSettings.timer2.singleShot,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem *clockSettingItemPointers[] =
	{
		&timeZoneSetting,
		&alarm1Hour,
		&alarm1Min,
		&alarm1Enabled,
		&alarm1TimeMatch,
		&alarm2Hour,
		&alarm2Min,
		&alarm2Enabled,
		&alarm2TimeMatch,
		&alarm3Hour,
		&alarm3Min,
		&alarm3Enabled,
		&alarm3TimeMatch,
		&interval1,
		&interval1Enabled,
		&interval1SingleShot,
		&interval2,
		&interval2Enabled,
		&interval2SingleShot};

struct SettingItemCollection clockSensorSettingItems = {
	"clock",
	"Alarm and interval timer settings",
	clockSettingItemPointers,
	sizeof(clockSettingItemPointers) / sizeof(struct SettingItem *)};

struct clockAlarmDescriptor
{
	struct ClockAlarm *alarm;
	int alarmMask;
};

struct clockAlarmDescriptor alarms[] = {
	{&clockSensorSettings.alarm1, ALARM1_TRIGGER},
	{&clockSensorSettings.alarm2, ALARM2_TRIGGER},
	{&clockSensorSettings.alarm3, ALARM3_TRIGGER}};

struct clockTimerDescriptor
{
	struct ClockTimer *timer;
	int timerMask;
};

struct clockTimerDescriptor timers[] = {
	{&clockSensorSettings.timer1, TIMER1_TRIGGER},
	{&clockSensorSettings.timer2, TIMER2_TRIGGER}};

void iterateThroughAlarms(void (*func)(clockAlarmDescriptor *alarm))
{
	for (unsigned int i = 0; i < sizeof(alarms) / sizeof(clockAlarmDescriptor); i = i + 1)
	{
		func(&alarms[i]);
	}
}

bool pastAlarmTime(struct ClockAlarm *alarm, struct clockReading *reading)
{
	Serial.printf("    Alarm:%d:%d  Time:%d:%d\n", alarm->hour, alarm->minute, reading->hour, reading->minute);

	if (reading->hour > alarm->hour)
	{
		return true;
	}

	if (reading->hour == alarm->hour)
	{
		if (reading->minute > alarm->minute)
		{
			return true;
		}
	}

	return false;
}

bool atAlarmTime(struct ClockAlarm *alarm, struct clockReading *reading)
{
	if (alarm->hour != reading->hour)
	{
		return false;
	}

	if (alarm->minute != reading->minute)
	{
		return false;
	}

	return true;
}

bool needToInitialiseAlarmsAndTimers;

void initialiseAlarm(clockAlarmDescriptor *alarmDesc, struct clockReading *reading)
{
	ClockAlarm *alarmDetails = alarmDesc->alarm;

	if (!alarmDetails->enabled)
	{
		return;
	}

	// allow the alarm to trigger during initialisation
	alarmDetails->triggered = false;

	if (!alarmDetails->fireOnTimeMatch)
	{
		// If the alarm time is less than the current time we fire the alarm anyway
		// This is so that if we start the device after an alarm time we still get
		// the alarm behaviour that we want
		if (pastAlarmTime(alarmDesc->alarm, reading))
		{
			alarmDesc->alarm->triggered = true;
			fireSensorListenersOnTrigger(&clockSensor, alarmDesc->alarmMask);
		}
	}
}

void initialiseTimer(clockTimerDescriptor *timerDesc, struct clockReading *reading)
{
	ClockTimer *timer = timerDesc->timer;

	timer->prevEnabled = timer->enabled;

	if (timer->enabled)
	{
		// need to set the time for the timer to fire to the time in the future
		int min, hour;
		min = reading->minute + timer->interval;
		hour = reading->hour;
		while (min >= 60)
		{
			min = min - 60;
			hour = hour + 1;
			if (hour == 24)
			{
				hour = 0;
			}
		}
		timer->nextMinute = min;
		timer->nextHour = hour;
		timer->triggered = false;
	}
}

void initialiseAlarms(struct clockReading *reading)
{
	for (unsigned int i = 0; i < sizeof(alarms) / sizeof(clockAlarmDescriptor); i = i + 1)
	{
		initialiseAlarm(&alarms[i], reading);
	}
}

void initialiseTimers(struct clockReading *reading)
{
	for (unsigned int i = 0; i < sizeof(timers) / sizeof(clockTimerDescriptor); i = i + 1)
	{
		initialiseTimer(&timers[i], reading);
	}
}

void checkAlarm(clockAlarmDescriptor *alarmDesc, struct clockReading *reading)
{
	ClockAlarm *alarmDetails = alarmDesc->alarm;

	if (atAlarmTime(alarmDesc->alarm, reading))
	{
		if (alarmDetails->triggered)
		{
			return;
		}
		alarmDetails->triggered = true;
		fireSensorListenersOnTrigger(&clockSensor, alarmDesc->alarmMask);
	}
	else
	{
		alarmDetails->triggered = false;
	}
}

void checkAlarms(struct clockReading *reading)
{
	for (unsigned int i = 0; i < sizeof(alarms) / sizeof(clockAlarmDescriptor); i = i + 1)
	{
		checkAlarm(&alarms[i], reading);
	}
}

bool timerExpired(struct ClockTimer *timer, struct clockReading *reading)
{
	if (timer->nextMinute != reading->minute)
	{
		return false;
	}

	if (timer->nextHour != reading->hour)
	{
		return false;
	}

	return true;
}

void checkTimer(clockTimerDescriptor *timerDesc, struct clockReading *reading)
{
	struct ClockTimer *timer = timerDesc->timer;

	if (timer->prevEnabled != timer->enabled)
	{
		// timer has changed state - may need to initalise it
		if (timer->enabled)
		{
			initialiseTimer(timerDesc, reading);
		}
		timer->prevEnabled = timer->enabled;
	}

	if (timerExpired(timer, reading))
	{
		if (timer->triggered)
		{
			return;
		}

		timer->triggered = true;

		fireSensorListenersOnTrigger(&clockSensor, timerDesc->timerMask);

		if (timer->singleShot)
		{
			timer->enabled = false;
		}
		else
		{
			initialiseTimer(timerDesc, reading);
		}
	}
	else
	{
		timer->triggered = false;
	}
}

void checkTimers(struct clockReading *reading)
{
	for (unsigned int i = 0; i < sizeof(timers) / sizeof(clockTimerDescriptor); i = i + 1)
	{
		checkTimer(&timers[i], reading);
	}
}

// Look for any listeners who want to get the time delivered to them as a string...

void checkClock(struct clockReading *reading)
{
	static int lastClockSecond = -1;
	static int lastClockMinute = -1;
	static int lastClockHour = -1;
	static int lastClockDay = -1;

	if (lastClockSecond == reading->second)
	{
		return;
	}

	lastClockSecond = reading->second;

	struct sensorListener *pos = clockSensor.listeners;

	while (pos != NULL)
	{
		char *messageBuffer = (char *)pos->config->optionBuffer + MESSAGE_START_POSITION;

		if (pos->config->sendOptionMask == CLOCK_SECOND_TICK)
		{
			TRACELN("Second Tick");
			snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%02d:%02d:%02d",
					 reading->hour,
					 reading->minute,
					 reading->second);
			pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
		}

		if (pos->config->sendOptionMask == CLOCK_MINUTE_TICK)
		{
			if (lastClockMinute != reading->minute)
			{
				TRACELN("Minute Tick");
				snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%02d:%02d", reading->hour, reading->minute);
				pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
				lastClockMinute = reading->minute;
			}
		}

		if (pos->config->sendOptionMask == CLOCK_HOUR_TICK)
		{
			if (lastClockHour != reading->hour)
			{
				TRACELN("Hour Tick");
				snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%02d:%02d", reading->hour, reading->minute);
				pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
				lastClockHour = reading->hour;
			}
		}

		if (pos->config->sendOptionMask == CLOCK_DAY_TICK)
		{
			if (lastClockDay != reading->day)
			{
				TRACELN("Day Tick");
				snprintf(messageBuffer, MAX_MESSAGE_LENGTH, "%02d:%02d:%02d", reading->day, reading->month, reading->year);
				pos->receiveMessage(pos->config->destination, pos->config->optionBuffer);
				lastClockDay = reading->day;
			}
		}

		pos = pos->nextMessageListener;
	}
}

struct sensorEventBinder ClockSensorListenerFunctions[] = {
	{"alarm1", ALARM1_TRIGGER},
	{"alarm2", ALARM2_TRIGGER},
	{"alarm3", ALARM3_TRIGGER},
	{"timer1", TIMER1_TRIGGER},
	{"timer2", TIMER2_TRIGGER},
	{"second", CLOCK_SECOND_TICK},
	{"minute", CLOCK_MINUTE_TICK},
	{"hour", CLOCK_HOUR_TICK},
	{"day", CLOCK_DAY_TICK}};

Timezone homeTimezone;

void startClockSensor()
{
	needToInitialiseAlarmsAndTimers = true;

	struct ClockReading *clockActiveReading;

	if (clockSensor.activeReading == NULL)
	{
		clockActiveReading = new ClockReading();
		clockSensor.activeReading = clockActiveReading;
	}
	else
	{
		clockActiveReading =
			(struct ClockReading *)clockSensor.activeReading;
	}

	if (WiFiProcessDescriptor.status != WIFI_OK)
	{
		clockSensor.status = CLOCK_ERROR_NO_WIFI;
		return;
	}

	events();

	if (waitForSync(CLOCK_SYNC_TIMEOUT))
	{
		clockSensor.status = SENSOR_OK;
		return;
	}

	clockSensor.status = CLOCK_ERROR_NEEDS_SYNC;
}

void stopClockSensor()
{
	clockSensor.status = CLOCK_STOPPED;
}

void getClockReadings()
{
	struct clockReading *clockActiveReading =
		(struct clockReading *)clockSensor.activeReading;

	clockActiveReading->hour = homeTimezone.hour();
	clockActiveReading->minute = homeTimezone.minute();
	clockActiveReading->second = homeTimezone.second();
	clockActiveReading->day = homeTimezone.day();
	clockActiveReading->month = homeTimezone.month();
	clockActiveReading->year = homeTimezone.year();
	clockActiveReading->dayOfWeek = homeTimezone.weekday();
	clockSensor.millisAtLastReading = millis();
}

void updateClockReading()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		clockSensor.status = CLOCK_ERROR_NO_WIFI;
	}

	switch (clockSensor.status)
	{

	case CLOCK_ERROR_NO_WIFI:

		if (WiFiProcessDescriptor.status == WIFI_OK)
		{
			clockSensor.status = CLOCK_ERROR_TIME_NOT_SET;
			homeTimezone.setLocation(F("Europe/London"));
		}
		break;

	case SENSOR_OK:
	case CLOCK_ERROR_TIME_NOT_SET:
	case CLOCK_ERROR_NEEDS_SYNC:

		events();

		switch (timeStatus())
		{
		case timeNotSet:
			clockSensor.status = CLOCK_ERROR_TIME_NOT_SET;
			break;
		case timeSet:
			getClockReadings();
			clockSensor.millisAtLastReading = millis();
			clockSensor.status = SENSOR_OK;
			break;
		case timeNeedsSync:
			if (waitForSync(CLOCK_SYNC_TIMEOUT))
			{
				getClockReadings();
				clockSensor.status = SENSOR_OK;
			}
			else
			{
				clockSensor.status = CLOCK_ERROR_NEEDS_SYNC;
			}
			break;
		}
		break;
	default:
		break;
	}

	if (clockSensor.status == SENSOR_OK)
	{
		struct clockReading *clockActiveReading;
		clockActiveReading =
			(struct clockReading *)clockSensor.activeReading;

		if (needToInitialiseAlarmsAndTimers)
		{
			initialiseAlarms(clockActiveReading);
			initialiseTimers(clockActiveReading);

			needToInitialiseAlarmsAndTimers = false;
		}

		checkAlarms(clockActiveReading);
		checkTimers(clockActiveReading);
		checkClock(clockActiveReading);
	}
}

void startClockSensorReading()
{
}

void addClockSensorReading(char *jsonBuffer, int jsonBufferSize)
{
	if (clockSensor.status == SENSOR_OK)
	{
		// struct clockReading *clockActiveReading;
		// clockActiveReading =
		// 	(struct clockReading *)clockSensor.activeReading;

		// snprintf(jsonBuffer, jsonBufferSize, "%s,\"timestamp\":\"%s %s %d %d %02d:%02d:%02d GMT+0000\"",
		// 	jsonBuffer,
		// 	dayNames[clockActiveReading->dayOfWeek],
		// 	monthNames[clockActiveReading->month],
		// 	clockActiveReading->day,
		// 	clockActiveReading->year,
		// 	clockActiveReading->hour,
		// 	clockActiveReading->minute,
		// 	clockActiveReading->second);

		snprintf(jsonBuffer, jsonBufferSize, "%s,\"timestamp\":\"%s\"",
				 jsonBuffer,
				 UTC.dateTime(RFC3339).c_str());
	}
}

bool getDateAndTime(char *buffer, int bufferLength)
{
	if (clockSensor.status == SENSOR_OK)
	{
		Serial.println("Got the data and time");
		struct clockReading *clockActiveReading;
		clockActiveReading =
			(struct clockReading *)clockSensor.activeReading;

		snprintf(buffer, bufferLength, "%s %s %d %d %02d:%02d:%02d ",
				 dayNames[clockActiveReading->dayOfWeek],
				 monthNames[clockActiveReading->month],
				 clockActiveReading->day,
				 clockActiveReading->year,
				 clockActiveReading->hour,
				 clockActiveReading->minute,
				 clockActiveReading->second);
		return true;
	}
	return false;
}

void clockSensorStatusMessage(char *buffer, int bufferLength)
{
	struct clockReading *clockActiveReading;
	clockActiveReading =
		(struct clockReading *)clockSensor.activeReading;

	switch (clockSensor.status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "Clock up to date: %s %s %d %d %02d:%02d:%02d ",
				 dayNames[clockActiveReading->dayOfWeek],
				 monthNames[clockActiveReading->month],
				 clockActiveReading->day,
				 clockActiveReading->year,
				 clockActiveReading->hour,
				 clockActiveReading->minute,
				 clockActiveReading->second);
		break;
	case CLOCK_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "Clock waiting for wifi");
		break;
	case CLOCK_ERROR_TIME_NOT_SET:
		snprintf(buffer, bufferLength, "Clock error time not set");
		break;
	case CLOCK_ERROR_NEEDS_SYNC:
		snprintf(buffer, bufferLength, "Clock waiting for sync");
		break;
	default:
		break;
	}
}

struct sensor clockSensor = {
	"clock",
	0, // millis at last reading
	0, // reading number
	0, // last transmitted reading number
	startClockSensor,
	stopClockSensor,
	updateClockReading,
	startClockSensorReading,
	addClockSensorReading,
	clockSensorStatusMessage,
	-1,	   // status
	false, // being updated
	NULL,  // active reading - set in setup
	0,	   // active time
	(unsigned char *)&clockSensorSettings,
	sizeof(struct ClockSensorSettings),
	&clockSensorSettingItems,
	NULL, // next active sensor
	NULL, // next all sensors
	NULL, // message listeners
	ClockSensorListenerFunctions,
	sizeof(ClockSensorListenerFunctions) / sizeof(struct sensorEventBinder)};
