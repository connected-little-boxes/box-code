#pragma once

#include <Arduino.h>
#include "settings.h"

#define ENV_READING_LIFETIME_MSECS 5000

#define BME280SENSOR_NOT_FITTED -1
#define BME280SENSOR_NOT_CONNECTED -2


//                876543210
//                     XXXX    sensor number (0x00-0x0F) mask 0000 1111 0x0F
//                YYYYY        trigger (0x10-0xF) mask 1111 0000 0x70

#define BME280_SENSOR_MASK 0x0F 
#define BME280_EVENT_MASK 0xFF0

#define BME280_HUMID 0x01
#define BME280_PRESS 0x02
#define BME280_TEMP 0x04			
#define BME280_ALL 0x08			

#define BME280_ON_SECOND 0x10      	
#define BME280_ON_MIN 0x20      	
#define BME280_ON_FIVE_MIN 0x40		
#define BME280_ON_HALF_HOUR 0x80	
#define BME280_ON_HOUR 0x100			
#define BME280_ON_CHANGE 0x200       

#define BME280SENSOR_SEND_HUMID_ON_SECOND (BME280_HUMID+BME280_ON_SECOND)
#define BME280SENSOR_SEND_HUMID_ON_MINUTE (BME280_HUMID+BME280_ON_MIN)
#define BME280SENSOR_SEND_HUMID_ON_FIVE_MINUTE (BME280_HUMID+BME280_ON_FIVE_MIN)
#define BME280SENSOR_SEND_HUMID_ON_HALF_HOUR (BME280_HUMID+BME280_ON_HALF_HOUR)
#define BME280SENSOR_SEND_HUMID_ON_HOUR (BME280_HUMID+BME280_ON_HOUR)
#define BME280SENSOR_SEND_HUMID_ON_CHANGE (BME280_HUMID+BME280_ON_CHANGE)

#define BME280SENSOR_SEND_TEMP_ON_SECOND (BME280_TEMP+BME280_ON_SECOND)
#define BME280SENSOR_SEND_TEMP_ON_MINUTE (BME280_TEMP+BME280_ON_MIN)
#define BME280SENSOR_SEND_TEMP_ON_FIVE_MINUTE (BME280_TEMP+BME280_ON_FIVE_MIN)
#define BME280SENSOR_SEND_TEMP_ON_HALF_HOUR (BME280_TEMP+BME280_ON_HALF_HOUR)
#define BME280SENSOR_SEND_TEMP_ON_HOUR (BME280_TEMP+BME280_ON_HOUR)
#define BME280SENSOR_SEND_TEMP_ON_CHANGE (BME280_TEMP+BME280_ON_CHANGE)

#define BME280SENSOR_SEND_PRESS_ON_SECOND (BME280_PRESS+BME280_ON_SECOND)
#define BME280SENSOR_SEND_PRESS_ON_MINUTE (BME280_PRESS+BME280_ON_MIN)
#define BME280SENSOR_SEND_PRESS_ON_FIVE_MINUTE (BME280_PRESS+BME280_ON_FIVE_MIN)
#define BME280SENSOR_SEND_PRESS_ON_HALF_HOUR (BME280_PRESS+BME280_ON_HALF_HOUR)
#define BME280SENSOR_SEND_PRESS_ON_HOUR (BME280_PRESS+BME280_ON_HOUR)
#define BME280SENSOR_SEND_PRESS_ON_CHANGE (BME280_PRESS+BME280_ON_CHANGE)

#define BME280SENSOR_SEND_ALL_ON_SECOND (BME280_ALL+BME280_ON_SECOND)
#define BME280SENSOR_SEND_ALL_ON_MINUTE (BME280_ALL+BME280_ON_MIN)
#define BME280SENSOR_SEND_ALL_ON_FIVE_MINUTE (BME280_ALL+BME280_ON_FIVE_MIN)
#define BME280SENSOR_SEND_ALL_ON_HALF_HOUR (BME280_ALL+BME280_ON_HALF_HOUR)
#define BME280SENSOR_SEND_ALL_ON_HOUR (BME280_ALL+BME280_ON_HOUR)

struct BME280SensorReading {
	int activeBMEAddress;
	float temperature;
	float pressure;
	float humidity;
	float temperatureAverage;
	float pressureAverage;
	float humidityAverage;
	// these are temporary values that are not for public use
	float temperatureTotal;
	float pressureTotal;
	float humidityTotal;
	float lastTempSent;
	float lastHumidSent;
	float lastPressSent;
	int envNoOfAveragesCalculated;
  int averageCount;
  unsigned long lastEnvqAverageMillis;
};


struct BME280SensorSettings {
	bool bme280SensorFitted;
	int envNoOfAverages;
	float tempDelta;
	float tempNormMin;
	float tempNormMax;
	float pressDelta;
	float pressNormMin;
	float pressNormMax;
	float humidDelta;
	float humidNormMin;
	float humidNormMax;
};

extern struct BME280SensorSettings bme280SensorSettings;

extern struct SettingItemCollection bme280SensorSettingItems;

int startbme280Sensor(struct sensor * bme280SensorSensor);
int stopbme280Sensor(struct sensor* bme280SensorSensor);
int updatebme280SensorReading(struct sensor * bme280SensorSensor);
void startbme280SensorReading(struct sensor * bme280SensorSensor);
int addbme280SensorReading(struct sensor * bme280SensorSensor, char * jsonBuffer, int jsonBufferSize);
void bme280SensorStatusMessage(struct sensor * bme280Sensorsensor, char * buffer, int bufferLength);
void bme280SensorTest();

extern struct sensor bme280Sensor;
