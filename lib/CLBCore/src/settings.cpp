#include <EEPROM.h>
#include <Arduino.h>
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

struct Device_Settings settings;

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

	if(!validateFloat(&value,newValueStr))
	{
		return false;
	}

	if(value<0 || value>1)
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

void printSettingValue(SettingItem *item)
{
	int *intValuePointer;
	boolean *boolValuePointer;
	float *floatValuePointer;
	double *doubleValuePointer;
	uint32_t *loraIDValuePointer;

	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	switch (item->settingType)
	{

	case text:
		Serial.println((char *)item->value);
		break;

	case password:
		//Serial.println((char *)item->value);
		Serial.println("******");
		break;

	case integerValue:
		intValuePointer = (int *)item->value;
		Serial.println(*intValuePointer);
		break;

	case floatValue:
		floatValuePointer = (float *)item->value;
		Serial.println(*floatValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		Serial.println(*doubleValuePointer);
		break;

	case yesNo:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			Serial.println("yes");
		}
		else
		{
			Serial.println("no");
		}
		break;

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		Serial.println(loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (uint32_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		Serial.println(loraKeyBuffer);
		break;

	default:
		Serial.println("***** invalid setting type");
	}
}

void printSetting(SettingItem *item)
{
	Serial.printf("    %s [%s]: ", item->prompt, item->formName);
	printSettingValue(item);
}

void dumpSetting(SettingItem *item)
{
	Serial.printf("%s=", item->formName);
	printSettingValue(item);
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

void appendSettingCollectionJson(SettingItemCollection *settings, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength,"%s[", buffer);

	for(int i=0; i< settings->noOfSettings;i++) {
		if(i>0){
			snprintf(buffer,bufferLength,"%s,",buffer );
		}
		appendSettingJSON(settings->settings[i], buffer,bufferLength);
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

void PrintSystemDetails()
{
	Serial.printf("   device:%s\n", settings.name);
}

void PrintAllSettings()
{
	PrintSystemDetails();
	Serial.println("Sensors");
	iterateThroughSensorSettingCollections(PrintSettingCollection);
	Serial.println("Processes");
	iterateThroughProcessSettingCollections(PrintSettingCollection);
}

void PrintSomeSettings(char *filter)
{
	settingsPrintFilter = filter;
	PrintSystemDetails();
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
	PrintSystemDetails();
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
	snprintf(settings.name, DEVICE_NAME_LENGTH, "CLB-%06lx", (unsigned long)PROC_ID);
	resetProcessesToDefaultSettings();
	resetSensorsToDefaultSettings();
	resetControllerListenersToDefaults();
}

unsigned char checksum;
int saveAddr;
int loadAddr;

void writeByteToEEPROM(byte b, int address)
{
	// Serial.printf("Writing byte: %d at address: %d\n", b, address); delay(1);
	checksum = checksum + b;

	if (EEPROM.read(address) != b)
	{
		EEPROM.write(address, b);
	}
}

void writeBytesToEEPROM(unsigned char *bytesToStore, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		byte b = *bytesToStore;

		writeByteToEEPROM(b, i);

		bytesToStore++;
	}
}

byte readByteFromEEPROM(int address)
{
	byte result;
	result = EEPROM.read(address);
	// Serial.printf("Reading byte: %d from address: %d\n", result, address); delay(1);

	checksum += result;
	return result;
}

void readBytesFromEEPROM(byte *destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		*destination = readByteFromEEPROM(i);
		destination++;
	}
}

void readSettingsBLockFromEEPROM(unsigned char *block, int size)
{
	readBytesFromEEPROM(block, loadAddr, size);
	loadAddr = loadAddr + size;
}

void saveSettingsBLockToEEPROM(unsigned char *block, int size)
{
	writeBytesToEEPROM(block, saveAddr, size);
	saveAddr = saveAddr + size;
}

void iterateThroughAllSettings(void (*func)(unsigned char *settings, int size))
{
	iterateThroughProcessSettings(func);

	iterateThroughSensorSettings(func);
}

void saveSettings()
{
	saveAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	saveSettingsBLockToEEPROM((unsigned char *)&settings, sizeof(struct Device_Settings));

	iterateThroughAllSettings(saveSettingsBLockToEEPROM);

	iterateThroughControllerListenerSettingCollections(saveSettingsBLockToEEPROM);

	writeByteToEEPROM(checksum, saveAddr);

	saveAddr = saveAddr+1;

	writeByteToEEPROM(CHECK_BYTE_O1, saveAddr);

	saveAddr = saveAddr+1;

	writeByteToEEPROM(CHECK_BYTE_O2, saveAddr);

	EEPROM.commit();
}

void loadSettings()
{
	loadAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	readSettingsBLockFromEEPROM((unsigned char *)&settings, sizeof(struct Device_Settings));

	iterateThroughAllSettings(readSettingsBLockFromEEPROM);

	iterateThroughControllerListenerSettingCollections(readSettingsBLockFromEEPROM);
}

void checksumBytesFromEEPROM(byte *destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		readByteFromEEPROM(i);
	}
}

void checkSettingsBLockFromEEPROM(unsigned char *block, int size)
{
	checksumBytesFromEEPROM(block, loadAddr, size);
	loadAddr = loadAddr + size;
}

boolean validStoredSettings()
{
	boolean result = true;
	
	loadAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	checkSettingsBLockFromEEPROM((unsigned char *)&settings, sizeof(struct Device_Settings));

	iterateThroughAllSettings(checkSettingsBLockFromEEPROM);

	iterateThroughControllerListenerSettingCollections(checkSettingsBLockFromEEPROM);

	unsigned char calcChecksum = checksum;

	unsigned char readChecksum = readByteFromEEPROM(loadAddr);

	Serial.printf("   settings occupy %d bytes of EEPROM\n", loadAddr - SETTINGS_EEPROM_OFFSET);

	if(calcChecksum != readChecksum)
	{
		Serial.printf("   checksum fail: Calc checksum:%02x  read checksum:%02x\n", calcChecksum, readChecksum);
		result = false;
	}

	loadAddr = loadAddr+1;

	byte check;
	
	check = readByteFromEEPROM(loadAddr);

	if(check != CHECK_BYTE_O1)
	{
		Serial.printf("   check byte 1 fail: Read:%02x expected:%02x\n", check, CHECK_BYTE_O1);
		result = false;
	}

	loadAddr = loadAddr+1;

	check = readByteFromEEPROM(loadAddr);

	if(check != CHECK_BYTE_O2)
	{
		Serial.printf("   check byte 2 fail: Read:%02x expected:%02x\n", check, CHECK_BYTE_O1);
		result = false;
	}

	return result;
}

boolean matchSettingCollectionName(SettingItemCollection *settingCollection, const char *name)
{
	int settingNameLength = strlen(settingCollection->collectionName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i] != settingCollection->collectionName[i]))
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
		if (tolower(name[i] != setting->formName[i]))
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

void testSettingsStorage()
{
	Serial.println("Testing Settings Storage");
	Serial.println("Resetting Settings");
	resetSettings();
	PrintAllSettings();
	Serial.println("Storing Settings");
	saveSettings();
	Serial.println("Restoring setings");
	saveSettings();
	Serial.println("Loading Settings");
	loadSettings();
	PrintAllSettings();
	if (validStoredSettings())
		Serial.println("Settings storage restored OK");
	else
		Serial.println("Something wrong with setting storage");

	settings.name[0] = 'x';

	if (!validStoredSettings())
		Serial.println("Settings change detected");
	else
		Serial.println("Settings change not detected");
}

void setupSettings()
{
	EEPROM.begin(EEPROM_SIZE);

	Serial.println("Settings Setup");

	//testSettingsStorage();

	if (validStoredSettings())
	{
		loadSettings();
		PrintSystemDetails();
		Serial.println("   settings loaded OK\n");
	}
	else
	{
		resetSettings();
		saveSettings();
		PrintSystemDetails();
		Serial.println("   ***** settings reset\n");
	}

	//PrintAllSettings();
}

