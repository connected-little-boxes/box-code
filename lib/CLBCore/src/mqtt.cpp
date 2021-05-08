
#include "mqtt.h"
#include "controller.h"

#include <PubSubClient.h>

struct MqttSettings mqttSettings;

void setDefaultMQTThost(void *dest)
{
	strcpy((char *)dest, DEFAULT_MQTT_HOST);
}

boolean validateMQTThost(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

void setDefaultMQTTport(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 1883; // use 8883 for secure MQTT connection
}

boolean validateMQTTtopic(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_TOPIC_LENGTH));
}

boolean validateMQTTtopicPrefix(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_TOPIC_PREFIX_LENGTH));
}

void setDefaultMQTTusername(void *dest)
{
	strcpy((char *)dest, DEFAULT_MQTT_USER);
}

void setDefaultMQTTpwd(void *dest)
{
	strcpy((char *)dest, DEFAULT_MQTT_PWD);
}

boolean validateMQTTusername(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_USER_NAME_LENGTH));
}

boolean validateMQTTPWD(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_PASSWORD_LENGTH));
}

void setDefaultMQTTTopicPrefix(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "lb");
}

void setDefaultMQTTpublishTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "data");
}

void setDefaultMQTTsubscribeTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "command");
}

void setDefaultMQTTreportTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "report");
}

void setDefaultMQTTsecsPerUpdate(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 360;
}

void setDefaultMQTTsecsPerRetry(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 10;
}

void setDefaultMQTTDeviceName(void *dest)
{
	char *destStr = (char *)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "CLB-%06lx", PROC_ID);
}

boolean validateMQTTDeviceName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, DEVICE_NAME_LENGTH));
}

struct SettingItem mqttDeviceNameSetting = {
	"MQTT Device name", "mqttdevicename", mqttSettings.mqttDeviceName, DEVICE_NAME_LENGTH, text, setDefaultMQTTDeviceName, validateMQTTDeviceName};

struct SettingItem mqttOnOffSetting = {
	"MQTT Active (yes or no)", "mqttactive", &mqttSettings.mqtt_enabled, ONOFF_INPUT_LENGTH, yesNo, setTrue, validateYesNo};

struct SettingItem mqttServerSetting = {
	"MQTT Host", "mqtthost", mqttSettings.mqttServer, SERVER_NAME_LENGTH, text, setDefaultMQTThost, validateServerName};

struct SettingItem mqttPortSetting = {
	"MQTT Port number", "mqttport", &mqttSettings.mqttPort, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTport, validateInt};

struct SettingItem mqttSecureSocketsSetting = {
	"MQTT Secure sockets active (yes or no)", "mqttsecure", &mqttSettings.mqttSecureSockets, YESNO_INPUT_LENGTH, yesNo, setFalse, validateYesNo};

struct SettingItem mqttUserSetting = {
	"MQTT UserName", "mqttuser", mqttSettings.mqttUser, MQTT_USER_NAME_LENGTH, text, setDefaultMQTTusername, validateMQTTusername};

struct SettingItem mqttPasswordSetting = {
	"MQTT Password", "mqttpwd", mqttSettings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setDefaultMQTTpwd, validateMQTTPWD};

struct SettingItem mqttTopicPrefixSetting = {
	"MQTT Topic prefix", "mqttpre", mqttSettings.mqttTopicPrefix, MQTT_TOPIC_PREFIX_LENGTH, text, setDefaultMQTTTopicPrefix, validateMQTTtopicPrefix};

struct SettingItem mqttPublishTopicSetting = {
	"MQTT Publish topic", "mqttpub", mqttSettings.mqttPublishTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTpublishTopic, validateMQTTtopic};

struct SettingItem mqttSubscribeTopicSetting = {
	"MQTT Subscribe topic", "mqttsub", mqttSettings.mqttSubscribeTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTsubscribeTopic, validateMQTTtopic};

struct SettingItem mqttReportTopicSetting = {
	"MQTT Reporting topic", "mqttreport", mqttSettings.mqttReportTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTreportTopic, validateMQTTtopic};

struct SettingItem mqttSecsPerUpdateSetting = {
	"MQTT Seconds per update", "mqttsecsperupdate", &mqttSettings.mqttSecsPerUpdate, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerUpdate, validateInt};

struct SettingItem seconds_per_mqtt_retrySetting = {
	"MQTT Seconds per retry", "mqttsecsperretry", &mqttSettings.seconds_per_mqtt_retry, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerRetry, validateInt};

struct SettingItem *mqttSettingItemPointers[] =
	{
		&mqttDeviceNameSetting,
		&mqttOnOffSetting,
		&mqttServerSetting,
		&mqttPortSetting,
		&mqttSecureSocketsSetting,
		&mqttUserSetting,
		&mqttPasswordSetting,
		&mqttTopicPrefixSetting,
		&mqttPublishTopicSetting,
		&mqttSubscribeTopicSetting,
		&mqttReportTopicSetting,
		&mqttSecsPerUpdateSetting,
		&seconds_per_mqtt_retrySetting};

struct SettingItemCollection mqttSettingItems = {
	"MQTT",
	"MQTT host, username, password and connection topics",
	mqttSettingItemPointers,
	sizeof(mqttSettingItemPointers) / sizeof(struct SettingItem *)};

unsigned long mqtt_timer_start;

PubSubClient *mqttPubSubClient = NULL;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean first_mqtt_message = true;

int messagesSent;
int messagesReceived;

void mqtt_deliver_command_result(char *result)
{
	publishBufferToMQTT(result);
}

// do not process incoming messages on this thread because it is a callback from the MQTT driver
// that might fire from a network interrupt

void callback(char *topic, byte *payload, unsigned int length)
{
	unsigned int i;

	for (i = 0; i < length; i++)
	{
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string
	mqtt_receive_buffer[i] = 0;

	messagesReceived++;
}

void clearIncomingMQTTMessage()
{
	mqtt_receive_buffer[0] = 0;
}

bool receivedIncomingMQTTMessage()
{
	return mqtt_receive_buffer[0] != 0;
}

void handleIncomingMQTTMessage()
{
	if (receivedIncomingMQTTMessage())
	{
		Serial.printf("Received from MQTT: %s\n", mqtt_receive_buffer);
		act_onJson_message(mqtt_receive_buffer, mqtt_deliver_command_result);
		clearIncomingMQTTMessage();
	}
}

int mqttConnectErrorNumber;
bool mqttStartCommandsPerformed ;

void initMQTT()
{
	MQTTProcessDescriptor.status = MQTT_OFF;
	mqttStartCommandsPerformed = false;
}

void startMQTT()
{
	if (mqttSettings.mqtt_enabled)
	{
		MQTTProcessDescriptor.status = MQTT_STARTING;
	}
	else
	{
		MQTTProcessDescriptor.status = MQTT_OFF;
	}
}

void restartMQTT()
{
	messagesReceived = 0;
	messagesSent = 0;
	clearIncomingMQTTMessage();

	if (mqttSettings.mqttServer[0]==0)
	{
		MQTTProcessDescriptor.status = MQTT_ERROR_NOT_CONFIGURED;
		return;
	}

	if (WiFiProcessDescriptor.status != WIFI_OK)
	{
		MQTTProcessDescriptor.status = MQTT_ERROR_NO_WIFI;
		return;
	}

	if (mqttPubSubClient == NULL)
	{
		if (mqttSettings.mqttSecureSockets)
		{
			WiFiClientSecure *secureClient = new WiFiClientSecure();
#if defined(ARDUINO_ARCH_ESP8266)
			secureClient->setInsecure();
#endif
			mqttPubSubClient = new PubSubClient(*secureClient);
		}
		else
		{
			WiFiClient *espClient = new WiFiClient();
			mqttPubSubClient = new PubSubClient(*espClient);
		}

		mqttPubSubClient->setBufferSize(MQTT_BUFFER_SIZE_MAX);

		mqttPubSubClient->setServer(mqttSettings.mqttServer, mqttSettings.mqttPort);
		mqttPubSubClient->setCallback(callback);
	}

	if (!mqttPubSubClient->connect(mqttSettings.mqttDeviceName, mqttSettings.mqttUser, mqttSettings.mqttPassword))
	{
		Serial.printf("Bad MQTT client state %d\n", mqttPubSubClient->state());

		displayMessage(MQTT_STATUS_BAD_STATE_MESSAGE_NUMBER, ledFlashAlertState, MQTT_STATUS_BAD_STATE_MESSAGE_TEXT);

		switch (mqttPubSubClient->state())
		{
		case MQTT_CONNECT_BAD_PROTOCOL:
			MQTTProcessDescriptor.status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_BAD_CLIENT_ID:
			MQTTProcessDescriptor.status = MQTT_ERROR_BAD_CLIENT_ID;
			break;
		case MQTT_CONNECT_UNAVAILABLE:
			MQTTProcessDescriptor.status = MQTT_ERROR_CONNECT_UNAVAILABLE;
			break;
		case MQTT_CONNECT_BAD_CREDENTIALS:
			MQTTProcessDescriptor.status = MQTT_ERROR_BAD_CREDENTIALS;
			break;
		case MQTT_CONNECT_UNAUTHORIZED:
			MQTTProcessDescriptor.status = MQTT_ERROR_CONNECT_UNAUTHORIZED;
			break;
		case MQTT_CONNECTION_TIMEOUT:
			MQTTProcessDescriptor.status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_FAILED:
			MQTTProcessDescriptor.status = MQTT_ERROR_CONNECT_FAILED;
			break;
		default:
			mqttConnectErrorNumber = mqttPubSubClient->state();
			MQTTProcessDescriptor.status = MQTT_ERROR_CONNECT_ERROR;
			break;
		}
		return;
	}

	char topicBuffer [MQTT_TOPIC_PREFIX_LENGTH+MQTT_TOPIC_LENGTH];

	snprintf(topicBuffer,MQTT_TOPIC_PREFIX_LENGTH+MQTT_TOPIC_LENGTH,"%s/%s/%s", mqttSettings.mqttTopicPrefix,mqttSettings.mqttSubscribeTopic,mqttSettings.mqttDeviceName);

	Serial.printf("Subscribing to:%s\n", topicBuffer);

	mqttPubSubClient->subscribe(topicBuffer);

	//snprintf(mqtt_send_buffer, MQTT_SEND_BUFFER_SIZE,
	//	"{\"dev\":\"%s\", \"status\":\"starting\"}",
	//	mqttSettings.name);

	//if (!mqttPubSubClient->publish(mqttSettings.mqttReportTopic, mqtt_send_buffer))
	//{
	//	Serial.println("publish failed");
	//	mqttProcess->status = MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//	return MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//}

	displayMessage(MQTT_STATUS_OK_MESSAGE_NUMBER, ledFlashNormalState, MQTT_STATUS_OK_MESSAGE_TEXT);

	MQTTProcessDescriptor.status = MQTT_OK;
}

int mqttRetries = 0;

int publishBufferToMQTTTopic(char *buffer, char *topic)
{

	if (MQTTProcessDescriptor.status == MQTT_OK)
	{
		messagesSent++;

		char topicBuffer [MQTT_TOPIC_PREFIX_LENGTH+MQTT_TOPIC_LENGTH];

		if( mqttSettings.mqttTopicPrefix[0]==0)
		{
			// no prefix - just send the topic
			snprintf(topicBuffer,MQTT_TOPIC_PREFIX_LENGTH+MQTT_TOPIC_LENGTH,"%s", topic);
		}
		else {
			// send the prefix separated from the topic by a /
			snprintf(topicBuffer,MQTT_TOPIC_PREFIX_LENGTH+MQTT_TOPIC_LENGTH,"%s/%s", mqttSettings.mqttTopicPrefix,topic);
		}

		Serial.printf("MQTT publishing:%s to topic:%s\n", buffer, topicBuffer);

		boolean result = mqttPubSubClient->publish(topicBuffer, buffer);

		if(result)
		{
			Serial.println();
			displayMessage(MQTT_STATUS_TRANSMIT_OK_MESSAGE_NUMBER, ledFlashNormalState, MQTT_STATUS_TRANSMIT_OK_MESSAGE_TEXT);
			return MQTT_STATUS_TRANSMIT_OK_MESSAGE_NUMBER;
		}
		else
		{
			Serial.println(" - Failed");
			displayMessage(MQTT_STATUS_PUBLISH_FAILED_MESSAGE_NUMBER, ledFlashAlertState, MQTT_STATUS_PUBLISH_FAILED_MESSAGE_TEXT);
			return MQTT_STATUS_PUBLISH_FAILED_MESSAGE_NUMBER;
		}
	}

	Serial.println("Not publishing message");

	displayMessage(MQTT_STATUS_MESSAGE_CANT_SEND_MESSAGE_NUMBER, ledFlashAlertState, MQTT_STATUS_MESSAGE_CANT_SEND_MESSAGE_TEXT);

	return MQTT_STATUS_MESSAGE_CANT_SEND_MESSAGE_NUMBER;
}

int publishCommandToRemoteDevice(char *buffer, char *remoteDeviceName)
{
	char topicBuffer [MQTT_TOPIC_LENGTH];

	snprintf(topicBuffer,MQTT_TOPIC_LENGTH,"%s/%s",mqttSettings.mqttSubscribeTopic,remoteDeviceName);

	return publishBufferToMQTTTopic(buffer, topicBuffer);
}


int publishBufferToMQTT(char *buffer)
{
	char topicBuffer [MQTT_TOPIC_LENGTH];

	snprintf(topicBuffer,MQTT_TOPIC_LENGTH,"%s/%s",mqttSettings.mqttPublishTopic,mqttSettings.mqttDeviceName);

	return publishBufferToMQTTTopic(buffer,topicBuffer);
}

void stopMQTT()
{
	// don't do anything because we are all going to die anyway
	MQTTProcessDescriptor.status = MQTT_OFF;
}

unsigned long timeOfLastMQTTsuccess = 0;

void updateMQTT()
{
	handleIncomingMQTTMessage();

	switch (MQTTProcessDescriptor.status)
	{

	case MQTT_OK:

		if (WiFi.status() != WL_CONNECTED)
		{
			MQTTProcessDescriptor.status = MQTT_ERROR_NO_WIFI;
			mqttPubSubClient->disconnect();
		}

		timeOfLastMQTTsuccess = millis();

		if (!mqttPubSubClient->loop())
		{
			mqttPubSubClient->disconnect();
			MQTTProcessDescriptor.status = MQTT_ERROR_LOOP_FAILED;
		}

		if(!mqttStartCommandsPerformed)
		{
			performCommandsInStore(MQTT_CONNECTED_COMMAND_STORE);
			mqttStartCommandsPerformed = true;
		}

		break;

	case MQTT_OFF:
		if (mqttSettings.mqtt_enabled)
			MQTTProcessDescriptor.status = MQTT_STARTING;
		break;

	case MQTT_STARTING:
		restartMQTT();
		break;

	case MQTT_ERROR_NOT_CONFIGURED:
		if (mqttSettings.mqttServer[0]!=0)
		{
			MQTTProcessDescriptor.status = MQTT_STARTING;
			return;
		}

	case MQTT_ERROR_NO_WIFI:
		if (WiFiProcessDescriptor.status == WIFI_OK)
		{
			restartMQTT();
		}
		break;

	case MQTT_ERROR_BAD_PROTOCOL:
	case MQTT_ERROR_BAD_CLIENT_ID:
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
	case MQTT_ERROR_BAD_CREDENTIALS:
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
	case MQTT_ERROR_CONNECT_FAILED:
	case MQTT_ERROR_CONNECT_ERROR:
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
	case MQTT_ERROR_LOOP_FAILED:
		if (ulongDiff(millis(), timeOfLastMQTTsuccess) > MQTT_CONNECT_RETRY_INTERVAL_MSECS)
		{
			restartMQTT();
			timeOfLastMQTTsuccess = millis();
		}
		break;

	default:
		break;
	}
}

bool MQTTStatusOK()
{
	return MQTTProcessDescriptor.status == MQTT_OK;
}

void mqttStatusMessage(char *buffer, int bufferLength)
{
	switch (MQTTProcessDescriptor.status)
	{
	case MQTT_OK:
		snprintf(buffer, bufferLength, "MQTT OK sent: %d rec: %d", messagesSent, messagesReceived);
		break;
	case MQTT_STARTING:
		snprintf(buffer, bufferLength, "MQTT Starting");
		break;
	case MQTT_ERROR_NOT_CONFIGURED:
		snprintf(buffer, bufferLength, "MQTT not configured");
		break;
	case MQTT_OFF:
		snprintf(buffer, bufferLength, "MQTT OFF");
		break;
	case MQTT_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "MQTT waiting for WiFi");
		break;
	case MQTT_ERROR_BAD_PROTOCOL:
		snprintf(buffer, bufferLength, "MQTT error bad protocol");
		break;
	case MQTT_ERROR_BAD_CLIENT_ID:
		snprintf(buffer, bufferLength, "MQTT error bad client ID");
		break;
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
		snprintf(buffer, bufferLength, "MQTT error connect unavailable");
		break;
	case MQTT_ERROR_BAD_CREDENTIALS:
		snprintf(buffer, bufferLength, "MQTT error bad credentials");
		break;
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
		snprintf(buffer, bufferLength, "MQTT error connect unauthorized");
		break;
	case MQTT_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect failed");
		break;
	case MQTT_ERROR_CONNECT_ERROR:
		snprintf(buffer, bufferLength, "MQTT error connect error %d", mqttConnectErrorNumber);
		break;
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect message failed");
		break;
	case MQTT_ERROR_LOOP_FAILED:
		snprintf(buffer, bufferLength, "MQTT error loop failed");
		break;
	default:
		snprintf(buffer, bufferLength, "MQTT failed but I'm not sure why: %d", MQTTProcessDescriptor.status);
		break;
	}
}

struct process MQTTProcessDescriptor = {
	"MQTT",
	initMQTT,
	startMQTT,
	updateMQTT,
	stopMQTT,
	MQTTStatusOK,
	mqttStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&mqttSettings, sizeof(MqttSettings), &mqttSettingItems,
	NULL,
	BOOT_PROCESS + ACTIVE_PROCESS,
	NULL,
	NULL,
	NULL,
	NULL, // no command options
	0	  // no command options
};
