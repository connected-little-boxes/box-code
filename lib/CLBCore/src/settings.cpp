#include <EEPROM.h>
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include "LittleFS.h"
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include "FS.h"
#endif

#include "string.h"
#include "errors.h"
#include "settings.h"
#include "debug.h"
#include "ArduinoJson-v5.13.2.h"
#include "utils.h"
#include "sensors.h"
#include "processes.h"
#include "controller.h"
#include "registration.h"
#include "HullOS.h"

// These functions are called to encrypt/decrypt fields of type password
// They are identical at the momement, but if you want to add some extra
// salt you can modify them accordingly.

#if defined(ARDUINO_ARCH_ESP32)

void encryptString(char *destination, int destLength, char *source)
{
	unsigned long seed = (PROC_ID + ENCRYPTION_SALT) % 0xFFFF ;
	//Serial.printf("Encrypting: %s seed:%lu\n", source, seed);

	localSrand(seed);
	int pos = 0;
	char *dest = destination;
	destLength = destLength - 1;
	while (*source)
	{
		int mask = (localRand()%30)+1;
		*dest = *source ^ mask;
		//Serial.printf("    mask:%d source:%c %d  dest:%c %d\n", mask, *source,*source, *dest, *dest);
		dest++;
		source++;
		pos++;
		if (pos == destLength)
		{
			break;
		}
	}
	*dest = 0;
	//Serial.printf("Output:%s\n", destination);
}

void decryptString(char *destination, int destLength, char *source)
{
	unsigned long seed = (PROC_ID + ENCRYPTION_SALT) % 0xFFFF ;
	//Serial.printf("Decrypting: %s seed:%lu\n", source, seed);

	localSrand(seed);
	int pos = 0;
	char *dest = destination;
	destLength = destLength - 1;
	while (*source)
	{
		int mask = (localRand()%30)+1;
		*dest = *source ^ mask;
		//Serial.printf("    mask:%d source:%c %d  dest:%c %d\n", mask, *source,*source, *dest, *dest);
		dest++;
		source++;
		pos++;
		if (pos == destLength)
		{
			break;
		}
	}

	*dest = 0;
	//Serial.printf("Output:%s\n", destination);
}

#endif

#if defined(ARDUINO_ARCH_ESP8266)

// use the original Arduino code here because existing devices 
// contain passwords encoded this way

void encryptString(char * destination, int destLength, char * source)
{
	randomSeed(PROC_ID+ENCRYPTION_SALT);
	int pos = 0;
	char * dest = destination;
	destLength= destLength -1;
	while(*source)
	{
		int mask = random(1,30);
		*dest = *source ^ mask;
		dest++;
		source++;
		pos++;
		if(pos==destLength)
		{
			break;
		}
	}
	*dest=0;
}

void decryptString(char * destination, int destLength, char * source)
{
	randomSeed(PROC_ID+ENCRYPTION_SALT);
	int pos = 0;
	char * dest = destination;
	destLength= destLength -1;
	while(*source)
	{
		int mask = random(1,30);
		*dest = *source ^ mask;
		dest++;
		source++;
		pos++;
		if(pos==destLength)
		{
			break;
		}
	}

	*dest=0;
}

#endif


void setEmptyString(void *dest)
{
	strcpy((char *)dest, "");
}

void setFalse(void *dest)
{
	boolean *destBool = (boolean *)dest;
	*destBool = false;
}

void setTrue(void *dest)
{
	boolean *destBool = (boolean *)dest;
	*destBool = true;
}

boolean validateOnOff(void *dest, const char *newValueStr)
{
	boolean *destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "on") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "off") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}

boolean validateYesNo(void *dest, const char *newValueStr)
{
	boolean *destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "yes") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "no") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}

boolean validateString(char *dest, const char *source, unsigned int maxLength)
{
	if (strlen(source) > (maxLength - 1))
		return false;

	strcpy(dest, source);

	return true;
}

boolean validateInt(void *dest, const char *newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	return false;
}

boolean validateUnsignedLong(void *dest, const char *newValueStr)
{
	unsigned long value;

	if (sscanf(newValueStr, "%lu", &value) == 1)
	{
		*(unsigned long *)dest = value;
		return true;
	}

	return false;
}

boolean validateDouble(void *dest, const char *newValueStr)
{
	double value;

	if (sscanf(newValueStr, "%lf", &value) == 1)
	{
		*(double *)dest = value;
		return true;
	}

	return false;
}

boolean validateFloat(void *dest, const char *newValueStr)
{
	float value;

	if (sscanf(newValueStr, "%f", &value) == 1)
	{
		*(float *)dest = value;
		return true;
	}

	return false;
}

boolean validateFloat0to1(void *dest, const char *newValueStr)
{
	float value;

	if (!validateFloat(&value, newValueStr))
	{
		return false;
	}

	if (value < 0 || value > 1)
	{
		return false;
	}

	*(float *)dest = value;
	return true;
}

void dump_hex(uint8_t *pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes
		if (*pos < 0x10)
		{
			TRACE("0");
		}
		TRACE_HEX(*pos);
		pos++;
		length--;
	}
	TRACELN();
}

char hex_digit(int val)
{
	if (val < 10)
	{
		return '0' + val;
	}
	else
	{
		return 'A' + (val - 10);
	}
}

void dumpHexString(char *dest, uint8_t *pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes

		*dest = hex_digit(*pos / 16);
		dest++;
		*dest = hex_digit(*pos % 16);
		dest++;
		pos++;
		length--;
	}
	*dest = 0;
}

void dumpUnsignedLong(char *dest, uint32_t value)
{
	// Write backwards to put least significant values in
	// the right place

	// move to the end of the string
	int pos = 8;
	// put the terminator in position
	dest[pos] = 0;
	pos--;

	while (pos > 0)
	{
		byte b = value & 0xff;
		dest[pos] = hex_digit(b % 16);
		pos--;
		dest[pos] = hex_digit(b / 16);
		pos--;
		value = value >> 8;
	}
}

int hexFromChar(char c, int *dest)
{
	if (c >= '0' && c <= '9')
	{
		*dest = (int)(c - '0');
		return WORKED_OK;
	}
	else
	{
		if (c >= 'A' && c <= 'F')
		{
			*dest = (int)(c - 'A' + 10);
			return WORKED_OK;
		}
		else
		{
			if (c >= 'a' && c <= 'f')
			{
				*dest = (int)(c - 'a' + 10);
				return WORKED_OK;
			}
		}
	}
	return INVALID_HEX_DIGIT_IN_VALUE;
}

#define MAX_DECODE_BUFFER_LENGTH 20

int decodeHexValueIntoBytes(uint8_t *dest, const char *newVal, int length)
{
	if (length > MAX_DECODE_BUFFER_LENGTH)
	{
		TRACELN("Incoming hex value will not fit in the buffer");
		return INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER;
	}

	// Each hex value is in two bytes - make sure the incoming text is the right length

	int inputLength = strlen(newVal);

	if (inputLength != length * 2)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	int8_t buffer[MAX_DECODE_BUFFER_LENGTH];
	int8_t *bpos = buffer;

	while (pos < inputLength)
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		*bpos = (int8_t)(d1 * 16 + d2);
		bpos++;
	}

	// If we get here the buffer has been filled OK

	memcpy_P(dest, buffer, length);
	return WORKED_OK;
}

int decodeHexValueIntoUnsignedLong(uint32_t *dest, const char *newVal)
{

	int inputLength = strlen(newVal);

	if (inputLength != 8)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	uint32_t result = 0;

	while (pos < inputLength)
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		uint32_t v = d1 * 16 + d2;
		result = result * 256 + v;
	}

	// If we get here the value has been received OK

	*dest = result;
	return WORKED_OK;
}

boolean validateDevName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, DEVICE_NAME_LENGTH));
}

boolean validateServerName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

void printSettingValue(SettingItem *item, char *buffer, int bufferLength)
{
	int *intValuePointer;
	boolean *boolValuePointer;
	float *floatValuePointer;
	double *doubleValuePointer;
	uint32_t *loraIDValuePointer;

	switch (item->settingType)
	{

	case text:
		snprintf(buffer, bufferLength, "%s", (char *)item->value);
		break;

	case password:
		snprintf(buffer, bufferLength, "*****");
		break;

	case integerValue:
		intValuePointer = (int *)item->value;
		snprintf(buffer, bufferLength, "%d", *intValuePointer);
		break;

	case floatValue:
		floatValuePointer = (float *)item->value;
		snprintf(buffer, bufferLength, "%f", *floatValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		snprintf(buffer, bufferLength, "%lf", *doubleValuePointer);
		break;

	case yesNo:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			snprintf(buffer, bufferLength, "yes");
		}
		else
		{
			snprintf(buffer, bufferLength, "no");
		}
		break;

	case loraKey:
		dumpHexString(buffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		break;

	case loraID:
		loraIDValuePointer = (uint32_t *)item->value;
		dumpUnsignedLong(buffer, *loraIDValuePointer);
		break;

	default:
		snprintf(buffer, bufferLength, "***** invalid setting type");
	}
}

void printSetting(SettingItem *item)
{
	char itemBuffer[SETTING_VALUE_OUTPUT_LENGTH];
	printSettingValue(item, itemBuffer, SETTING_VALUE_OUTPUT_LENGTH);

	Serial.printf("    %s [%s]: %s\n", item->prompt, item->formName, itemBuffer);
}

void dumpSetting(SettingItem *item)
{
	char itemBuffer[SETTING_VALUE_OUTPUT_LENGTH];
	printSettingValue(item, itemBuffer, SETTING_VALUE_OUTPUT_LENGTH);

	Serial.printf("%s=%s\n", item->formName, itemBuffer);
}

File saveFile;
File loadFile;

void saveSettingToFile(SettingItem *item)
{
	char itemBuffer[SETTING_VALUE_OUTPUT_LENGTH];

	if (item->settingType == password)
	{
		// need to encrypt the setting item
		encryptString(itemBuffer, SETTING_VALUE_OUTPUT_LENGTH,
					  (char *)item->value);
	}
	else
	{
		printSettingValue(item, itemBuffer, SETTING_VALUE_OUTPUT_LENGTH);
	}
	saveFile.printf("%s=%s\n", item->formName, itemBuffer);
}

void saveSettingCollectionToFile(SettingItemCollection *settingCollection)
{
	TRACE("  Saving setting collection: ");
	TRACELN(settingCollection->collectionName);
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		saveSettingToFile(settingCollection->settings[settingNo]);
	}
}

void saveAllSettingsToFile(char *path)
{
	TRACE("Saving all settings to the file:");
	TRACELN(path);
	saveFile = LittleFS.open(path, "w");
	iterateThroughSensorSettingCollections(saveSettingCollectionToFile);
	iterateThroughProcessSettingCollections(saveSettingCollectionToFile);
	saveFile.close();
	TRACELN("Settings saved");
}

processSettingCommandResult decodeSettingCommand(char *commandStart)
{
	char *command = (char *)commandStart;
	SettingItem *setting = findSettingByName(command);

	if (setting != NULL)
	{
		// found a setting - get the length of the setting name:
		int settingNameLength = strlen(setting->formName);

		if (command[settingNameLength] == 0)
		{
			// Settting is on it's own on the line
			// Just print the value
			printSetting(setting);
			return displayedOK;
		}

		if (command[settingNameLength] == '=')
		{
			// Setting is being assigned a value
			// move down the input to the new value
			char *startOfSettingInfo = command + settingNameLength + 1;

			if (setting->settingType == password)
			{
				// need to decode passwords
				decryptString((char *)setting->value, setting->maxLength, startOfSettingInfo);
				return setOK;
			}

			// use setting validation behaviour

			if (setting->validateValue(setting->value, startOfSettingInfo))
			{
				return setOK;
			}

			return settingValueInvalid;
		}
	}
	return settingNotFound;
}

bool loadAllSettingsFromFile(char *path)
{
	TRACE("Loading all settings from the file:");
	TRACELN(path);

	loadFile = LittleFS.open(path, "r");

	if (!loadFile || loadFile.isDirectory())
	{
		TRACELN("  failed to open the file");
		return false;
	}

	while (loadFile.available())
	{
		String line = loadFile.readStringUntil('\n');

		const char *lineChar = line.c_str();

		if (decodeSettingCommand((char *)lineChar) != setOK)
		{
			TRACE("  bad setting:");
			TRACELN(lineChar);
		}
	}

	loadFile.close();
	TRACELN("Settings loaded successfully");
	return true;
}

void appendSettingJSON(SettingItem *item, char *jsonBuffer, int bufferLength)
{
	int *intValuePointer;
	boolean *boolValuePointer;
	float *floatValuePointer;
	double *doubleValuePointer;
	uint32_t *loraIDValuePointer;

	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	snprintf(jsonBuffer, bufferLength,
			 "%s\"%s\":",
			 jsonBuffer,
			 item->formName);

	switch (item->settingType)
	{

	case text:
		snprintf(jsonBuffer, bufferLength,
				 "%s\"%s\"",
				 jsonBuffer,
				 (char *)item->value);
		break;

	case password:
		snprintf(jsonBuffer, bufferLength,
				 "%s\"******\"",
				 jsonBuffer);
		break;

	case integerValue:
		intValuePointer = (int *)item->value;
		snprintf(jsonBuffer, bufferLength,
				 "%s%d",
				 jsonBuffer,
				 *intValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		snprintf(jsonBuffer, bufferLength,
				 "%s%lf",
				 jsonBuffer,
				 *doubleValuePointer);
		break;

	case floatValue:
		floatValuePointer = (float *)item->value;
		snprintf(jsonBuffer, bufferLength,
				 "%s%f",
				 jsonBuffer,
				 *floatValuePointer);
		break;

	case yesNo:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			snprintf(jsonBuffer, bufferLength,
					 "%syes",
					 jsonBuffer);
		}
		else
		{
			snprintf(jsonBuffer, bufferLength,
					 "%sno",
					 jsonBuffer);
		}
		break;

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		snprintf(jsonBuffer, bufferLength,
				 "%s\"%s\"",
				 jsonBuffer,
				 loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (uint32_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		snprintf(jsonBuffer, bufferLength,
				 "%s\"%s\"",
				 jsonBuffer,
				 loraKeyBuffer);
		break;

	default:
		snprintf(jsonBuffer, bufferLength,
				 "%s\"******Invalid setting type\"",
				 jsonBuffer);
	}
}

void resetSetting(SettingItem *setting)
{
	setting->setDefault(setting->value);
}

void resetSettingCollection(SettingItemCollection *settingCollection)
{
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		resetSetting(settingCollection->settings[settingNo]);
	}
}

void PrintSettingCollection(SettingItemCollection *settingCollection)
{
	Serial.printf("\n%s\n", settingCollection->collectionName);
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		printSetting(settingCollection->settings[settingNo]);
	}
}

void appendSettingCollectionJson(SettingItemCollection *settings, char *buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "%s[", buffer);

	for (int i = 0; i < settings->noOfSettings; i++)
	{
		if (i > 0)
		{
			snprintf(buffer, bufferLength, "%s,", buffer);
		}
		appendSettingJSON(settings->settings[i], buffer, bufferLength);
	}

	snprintf(buffer, bufferLength, "%s]", buffer);
}

// This is using a global value to feed into a function. So sue me.

char *settingsPrintFilter;

void PrintSettingCollectionFiltered(SettingItemCollection *settingCollection)
{
	bool gotSetting = false;

	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		if (strContains(settingCollection->settings[settingNo]->formName, settingsPrintFilter))
		{
			gotSetting = true;
			break;
		}
	}

	if (gotSetting)
	{
		Serial.printf("\n%s\n", settingCollection->collectionName);
		for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
		{
			if (strContains(settingCollection->settings[settingNo]->formName, settingsPrintFilter))
			{
				printSetting(settingCollection->settings[settingNo]);
			}
		}
	}
}

void PrintSystemDetails(char *buffer, int length)
{
	snprintf(buffer, length, "CLB-%06lx", (unsigned long)PROC_ID);
}

void PrintAllSettings()
{
	char deviceNameBuffer[DEVICE_NAME_LENGTH];
	PrintSystemDetails(deviceNameBuffer, DEVICE_NAME_LENGTH);

	Serial.println(deviceNameBuffer);
	Serial.println("Sensors");
	iterateThroughSensorSettingCollections(PrintSettingCollection);
	Serial.println("Processes");
	iterateThroughProcessSettingCollections(PrintSettingCollection);
}

void PrintSomeSettings(char *filter)
{
	settingsPrintFilter = filter;
	char deviceNameBuffer[DEVICE_NAME_LENGTH];
	PrintSystemDetails(deviceNameBuffer, DEVICE_NAME_LENGTH);
	Serial.println(deviceNameBuffer);
	Serial.println("Sensors");
	iterateThroughSensorSettingCollections(PrintSettingCollectionFiltered);
	Serial.println("Processes");
	iterateThroughProcessSettingCollections(PrintSettingCollectionFiltered);
}

void printSettingStorage(sensor *sensor)
{
	Serial.printf("   %s Setting Storage:%d\n", sensor->sensorName, sensor->settingsStoreLength);
}

void printProcessStorage(process *process)
{
	Serial.printf("   %s Setting Storage:%d Command parameter Storage:%d\n",
				  process->processName, process->settingsStoreLength, process->commandItemSize);
}

void PrintStorage()
{
	char deviceNameBuffer[DEVICE_NAME_LENGTH];
	PrintSystemDetails(deviceNameBuffer, DEVICE_NAME_LENGTH);
	Serial.println(deviceNameBuffer);
	Serial.println("Sensors");
	iterateThroughSensors(printSettingStorage);
	Serial.println("Processes");
	iterateThroughAllProcesses(printProcessStorage);
}

void DumpSettingCollection(SettingItemCollection *settingCollection)
{
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		dumpSetting(settingCollection->settings[settingNo]);
	}
}

void DumpSettingCollectionFiltered(SettingItemCollection *settingCollection)
{
	bool gotSetting = false;

	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		if (strContains(settingCollection->settings[settingNo]->formName, settingsPrintFilter))
		{
			gotSetting = true;
			break;
		}
	}

	if (gotSetting)
	{
		for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
		{
			if (strContains(settingCollection->settings[settingNo]->formName, settingsPrintFilter))
			{
				dumpSetting(settingCollection->settings[settingNo]);
			}
		}
	}
}

void DumpAllSettings()
{
	Serial.println("\n");
	iterateThroughSensorSettingCollections(DumpSettingCollection);
	iterateThroughProcessSettingCollections(DumpSettingCollection);
}

void DumpSomeSettings(char *filter)
{
	settingsPrintFilter = filter;
	Serial.println("\n");
	iterateThroughSensorSettingCollections(DumpSettingCollectionFiltered);
	iterateThroughProcessSettingCollections(DumpSettingCollectionFiltered);
}

void resetSettings()
{
	// PROC_ID is defined in utils.h
	resetProcessesToDefaultSettings();
	resetSensorsToDefaultSettings();
	resetControllerListenersToDefaults();
}

void iterateThroughAllSettings(void (*func)(unsigned char *settings, int size))
{
	iterateThroughProcessSettings(func);

	iterateThroughSensorSettings(func);
}

void saveSettings()
{
	saveAllSettingsToFile(SETTINGS_FILENAME);
}

bool loadSettings()
{
	return loadAllSettingsFromFile(SETTINGS_FILENAME);
}

boolean matchSettingCollectionName(SettingItemCollection *settingCollection, const char *name)
{
	int settingNameLength = strlen(settingCollection->collectionName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i]) != tolower(settingCollection->collectionName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	return false;
}

SettingItemCollection *findSettingItemCollectionByName(const char *name)
{
	sensor *s = findSensorSettingCollectionByName(name);
	if (s != NULL)
		return s->settingItems;

	process *p = findProcessSettingCollectionByName(name);
	if (p != NULL)
		return p->settingItems;

	return NULL;
}

boolean matchSettingName(SettingItem *setting, const char *name)
{
	int settingNameLength = strlen(setting->formName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i]) != tolower(setting->formName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	if (name[settingNameLength] == '=')
		return true;

	return false;
}

SettingItem *findSettingByNameInCollection(SettingItemCollection settingCollection, const char *name)
{
	for (int settingNo = 0; settingNo < settingCollection.noOfSettings; settingNo++)
	{
		if (matchSettingName(settingCollection.settings[settingNo], name))
			return settingCollection.settings[settingNo];
	}
	return NULL;
}

SettingItem *findSettingByName(const char *settingName)
{
	SettingItem *result;

	result = FindSensorSettingByFormName(settingName);

	if (result != NULL)
		return result;

	result = FindProcesSettingByFormName(settingName);
	if (result != NULL)
		return result;

	return NULL;
}

processSettingCommandResult processSettingCommand(char *commandStart)
{
	char *command = (char *)commandStart;
	SettingItem *setting = findSettingByName(command);

	if (setting != NULL)
	{
		// found a setting - get the length of the setting name:
		int settingNameLength = strlen(setting->formName);

		if (command[settingNameLength] == 0)
		{
			// Settting is on it's own on the line
			// Just print the value
			printSetting(setting);
			return displayedOK;
		}

		if (command[settingNameLength] == '=')
		{
			// Setting is being assigned a value
			// move down the input to the new value
			char *startOfSettingInfo = command + settingNameLength + 1;
			if (setting->validateValue(setting->value, startOfSettingInfo))
			{
				return setOK;
			}
			return settingValueInvalid;
		}
	}
	return settingNotFound;
}

void sendSettingItemToJSONString(struct SettingItem *item, char *buffer, int bufferSize)
{
	int *valuePointer;
	float *floatPointer;
	double *doublePointer;
	boolean *boolPointer;
	uint32_t *loraIDValuePointer;
	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	switch (item->settingType)
	{
	case text:
		snprintf(buffer, bufferSize, "\"%s\"", (char *)item->value);
		break;
	case password:
		//snprintf(buffer, bufferSize, "\"%s\"", item->value);
		snprintf(buffer, bufferSize, "\"******\"");
		break;
	case integerValue:
		valuePointer = (int *)item->value;
		snprintf(buffer, bufferSize, "%d", *valuePointer);
		break;
	case floatValue:
		floatPointer = (float *)item->value;
		snprintf(buffer, bufferSize, "%f", *floatPointer);
		break;
	case doubleValue:
		doublePointer = (double *)item->value;
		snprintf(buffer, bufferSize, "%lf", *doublePointer);
		break;
	case yesNo:
		boolPointer = (boolean *)item->value;
		if (*boolPointer)
		{
			snprintf(buffer, bufferSize, "\"yes\"");
		}
		else
		{
			snprintf(buffer, bufferSize, "\"no\"");
		}
		break;
	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		snprintf(buffer, bufferSize, "\"%s\"", loraKeyBuffer);
		break;
	case loraID:
		loraIDValuePointer = (uint32_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		snprintf(buffer, bufferSize, "\"%s\"", loraKeyBuffer);
		break;
	default:
		snprintf(buffer, bufferSize, "\"***invalid setting type\"");
	}
}

void setupSettings()
{
	TRACELN("Setting up settings");

#if defined(ARDUINO_ARCH_ESP8266)
	if (!LittleFS.begin())
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
	}
#endif

#if defined(ARDUINO_ARCH_ESP32)
	if (!LittleFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
	}

#endif

	resetSettings();

	if (loadSettings())
	{
		Serial.println("Settings loaded OK");
	}
	else
	{
		Serial.println("Settings reset to default values");
		resetSettings();
	}

	char deviceNameBuffer[DEVICE_NAME_LENGTH];
	PrintSystemDetails(deviceNameBuffer, DEVICE_NAME_LENGTH);
	Serial.println(deviceNameBuffer);
}
