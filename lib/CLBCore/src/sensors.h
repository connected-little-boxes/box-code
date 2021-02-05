#pragma once

#include <Arduino.h>
#include "settings.h"
#include "controller.h"

#define SENSOR_OK 0
#define SENSOR_OFF 1

#define CONTROLLER_NO_OF_LISTENERS 10
#define CONTROLLER_COMMAND_LENGTH 150

#define LISTENER_NAME_LENGTH 30
#define COMMAND_PROCESS_NAME_LENGTH 15
#define COMMAND_NAME_LENGTH 20
#define SENSOR_NAME_LENGTH 10
#define DESTINATION_NAME_LENGTH 30
#define OPTION_STRING_LENGTH 20

#define OPTION_STORAGE_SIZE 100

// Received from MQTT and stored in settings - used to build commandMessageListener
struct sensorListenerConfiguration{
	char commandProcess [COMMAND_PROCESS_NAME_LENGTH];  // the process containing the command to be performed
	char commandName[COMMAND_NAME_LENGTH];              // the command to be performed
	char listenerName [LISTENER_NAME_LENGTH]; // maps onto a command provided by this sensor 
	char sensorName [SENSOR_NAME_LENGTH];   // name of the sensor providing the command
	char destination [DESTINATION_NAME_LENGTH];  // destination field, usually used for MQTT publishing
	unsigned char optionBuffer [OPTION_STORAGE_SIZE];
	int sendOptionMask;                             // mask of bits that determine when a sensor will deliver to the listener
	                                                // the bits are different for each sensor
	int readingIntervalSecs;                        // interval between reading transmissions 
	                                               // if sendOnChange is set the device will not send readings
												   // if changes are detected within this interval. 0 means that 
												   // all changes will be sent.
};

struct sensorListener{
	struct sensor * sensor;
	struct sensorListenerConfiguration * config;
	unsigned long lastReadingMillis;
	int (*receiveMessage)(char * destination, unsigned char * options);
	struct sensorListener * nextMessageListener;
};

struct sensorEventBinder{
	char * listenerName;
	int optionMask;
};

struct sensor
{
	char * sensorName;
	unsigned long millisAtLastReading;
	int readingNumber;
	int lastTransmittedReadingNumber;
	void(*startSensor)();
	void(*stopSensor)();
	void(*updateSensor)();
	void(*startReading)();
	void(*addReading)(char * jsonBuffer, int jsonBufferSize);
	void(*getStatusMessage)(char * buffer, int bufferLength);
	int status;      // zero means OK - any other value is an error state
	boolean beingUpdated;  // active means that the sensor will be updated 
	void * activeReading;
	unsigned int activeTime;
	unsigned char* settingsStoreBase;
	int settingsStoreLength;
	struct SettingItemCollection* settingItems;
	struct sensor * nextActiveSensor;
	struct sensor * nextAllSensors;
	struct sensorListener * listeners;
	struct sensorEventBinder * sensorListenerFunctions;
	int noOfSensorListenerFunctions;
};

void addSensorToAllSensorsList(struct sensor *newSensor);
void addSensorToActiveSensorsList(struct sensor *newSensor);

struct sensor * findSensorByName( const char * name);
struct sensor * findSensorSettingCollectionByName(const char * name);
void startSensors();
void dumpSensorStatus();
void startSensorsReading();
void updateSensors();
void createSensorJson(char * deviceName, char * buffer, int bufferLength);
void displaySensorStatus();
void stopSensors();
void iterateThroughSensors (void (*func) (sensor * s) );
void iterateThroughSensorSettingCollections(void (*func) (SettingItemCollection* s));
void iterateThroughSensorSettings(void (*func) (unsigned char* settings, int size));
void iterateThroughSensorSettings(void (*func) (SettingItem* s));
void resetSensorsToDefaultSettings();
SettingItem* FindSensorSettingByFormName(const char* settingName);
void addMessageListenerToSensor(struct sensor *sensor, struct sensorListener * listener);
void iterateThroughSensorListeners(struct sensor * sensor, void (*func) (struct sensorListner * listener));
void fireSensorListenersOnMaskBit(struct sensor *sensor, int mask);
struct sensorEventBinder *findSensorListenerByName(struct sensor *s, const char *name);
