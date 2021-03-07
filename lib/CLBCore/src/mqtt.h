#pragma once

#include "utils.h"
#include "processes.h"
#include "settings.h"
#include "messages.h"
#include "connectwifi.h"

#define MQTT_OK 700
#define MQTT_OFF 701
#define MQTT_STARTING 702
#define MQTT_ERROR_NO_WIFI 703
#define MQTT_ERROR_BAD_PROTOCOL 704
#define MQTT_ERROR_BAD_CLIENT_ID 705
#define MQTT_ERROR_CONNECT_UNAVAILABLE -706
#define MQTT_ERROR_BAD_CREDENTIALS 707
#define MQTT_ERROR_CONNECT_UNAUTHORIZED -708
#define MQTT_ERROR_CONNECT_FAILED 709
#define MQTT_ERROR_CONNECT_ERROR 710
#define MQTT_ERROR_CONNECT_MESSAGE_FAILED 711
#define MQTT_ERROR_LOOP_FAILED 712
#define MQTT_ERROR_NOT_CONFIGURED 713

#define MQTT_CONNECT_RETRY_INTERVAL_MSECS 60000

#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_TOPIC_LENGTH 150

#define MQTT_NO_OF_RETRIES 3

#define MQTT_STATUS_OK_MESSAGE_NUMBER 2
#define MQTT_STATUS_OK_MESSAGE_TEXT "Mqtt OK"

#define MQTT_STATUS_TRANSMIT_OK_MESSAGE_NUMBER 21
#define MQTT_STATUS_TRANSMIT_OK_MESSAGE_TEXT "Mqtt transmit OK"

#define MQTT_STATUS_TRANSMIT_TIMOUT_MESSAGE_NUMBER 22
#define MQTT_STATUS_TRANSMIT_TIMEOUT_MESSAGE_TEXT "Mqtt transmit timeout"

#define MQTT_STATUS_BAD_STATE_MESSAGE_NUMBER 23
#define MQTT_STATUS_BAD_STATE_MESSAGE_TEXT "Mqtt bad state"



#define MQTT_BUFFER_SIZE_MAX 1000

struct MqttSettings
{
	char mqttDeviceName[DEVICE_NAME_LENGTH];
	char mqttServer[SERVER_NAME_LENGTH];
	boolean mqttSecureSockets;
	int mqttPort;
	char mqttUser[MQTT_USER_NAME_LENGTH];
	char mqttPassword[MQTT_PASSWORD_LENGTH];
	char mqttPublishTopic[MQTT_TOPIC_LENGTH];
	char mqttSubscribeTopic[MQTT_TOPIC_LENGTH];
	char mqttReportTopic[MQTT_TOPIC_LENGTH];

	int mqttSecsPerUpdate;
	int seconds_per_mqtt_retry;
	boolean mqtt_enabled;
};

extern struct MqttSettings mqttSettings;

extern struct SettingItemCollection mqttSettingItems;

boolean publishBufferToMQTT(char * buffer);
boolean publishBufferToMQTTTopic(char *buffer, char * topic);

boolean validateMQTTtopic(void *dest, const char *newValueStr);

extern struct process MQTTProcessDescriptor;


