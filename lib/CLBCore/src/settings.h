#pragma once

#include <Arduino.h>

#ifdef DEFAULTS_ON

#include "defaults.hsec"

#else

#define DEFAULT_WIFI1_SSID ""
#define DEFAULT_WIFI1_PWD ""
#define DEFAULT_MQTT_HOST ""
#define DEFAULT_MQTT_USER ""
#define DEFAULT_MQTT_PWD ""

#endif

#include "utils.h"
#define Version "1.0.0.5"

// Sensor settings
#define UNKNOWN_SENSOR 0
#define SDS011_SENSOR 1
#define ZPH01_SENSOR 2
#define PMS5003_SENSOR 3

#define DEVICE_NAME_LENGTH 20

#define WIFI_SSID_LENGTH 30
#define WIFI_PASSWORD_LENGTH 30

#define NUMBER_INPUT_LENGTH 20
#define YESNO_INPUT_LENGTH 0
#define ONOFF_INPUT_LENGTH 0
#define SETTING_ERROR_MESSAGE_LENGTH 120
#define SERVER_NAME_LENGTH 200

#define LORA_KEY_LENGTH 16
#define LORA_EUI_LENGTH 8

#define MAX_SETTING_LENGTH 300

#define EEPROM_SIZE 5000
#define SETTINGS_EEPROM_OFFSET 0
#define CHECK_BYTE_O1 0x55
#define CHECK_BYTE_O2 0xAA

struct Device_Settings
{
	char name[DEVICE_NAME_LENGTH];
};

extern struct Device_Settings settings;

enum Setting_Type { text, password, integerValue, doubleValue, floatValue, loraKey, loraID, yesNo };

struct SettingItem {
	char * prompt;
	char * formName;
	void * value;
	int maxLength;
	Setting_Type settingType;
	void (*setDefault)(void * destination);
	boolean (*validateValue)(void * dest, const char * newValueStr);
};

struct SettingItemCollection
{
	char * collectionName;
	char * collectionDescription;
	SettingItem ** settings;
	int noOfSettings;
};

enum processSettingCommandResult { displayedOK, setOK, settingNotFound, settingValueInvalid };

void saveSettings();
void loadSettings();
void resetSettings();
void PrintAllSettings();
void PrintSomeSettings(char * filter);
void DumpAllSettings();
void DumpSomeSettings(char * filter);
void PrintStorage();

SettingItem* findSettingByName(const char* settingName);

SettingItemCollection * findSettingItemCollectionByName(const char * name);
boolean matchSettingCollectionName(SettingItemCollection* settingCollection, const char* name);
boolean matchSettingName(SettingItem* setting, const char* name);
processSettingCommandResult processSettingCommand(char * command);

void setupSettings();

void PrintSystemDetails();

void dumpHexString(char *dest, uint8_t *pos, int length);
void dumpUnsignedLong(char *dest, uint32_t value);
int decodeHexValueIntoBytes(uint8_t *dest, const char *newVal, int length);
int decodeHexValueIntoUnsignedLong(uint32_t *dest, const char *newVal);

void sendSettingItemToJSONString(struct SettingItem *item, char *buffer, int bufferSize);
void appendSettingCollectionJson(SettingItemCollection *settings, char * messageBuffer, int CONNECTION_MESSAGE_BUFFER_SIZE);

void appendSettingJSON(SettingItem *item, char *jsonBuffer, int bufferLength);


void setEmptyString(void *dest);

void setFalse(void *dest);

void setTrue(void *dest);

boolean validateOnOff(void *dest, const char *newValueStr);

boolean validateYesNo(void *dest, const char *newValueStr);

boolean validateString(char *dest, const char *source, unsigned int maxLength);

boolean validateInt(void *dest, const char *newValueStr);

boolean validateUnsignedLong(void *dest, const char *newValueStr);

boolean validateDouble(void *dest, const char *newValueStr);

boolean validateFloat(void *dest, const char *newValueStr);

boolean validateFloat0to1(void *dest, const char *newValueStr);

boolean validateDevName(void* dest, const char* newValueStr);

boolean validateServerName(void* dest, const char* newValueStr);
