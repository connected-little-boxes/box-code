#include "debug.h"

#include "controller.h"
#include "processes.h"
#include "console.h"
#include "mqtt.h"
#include "debug.h"
#include "utils.h"
#include "errors.h"
#include "settings.h"
#include "otaupdate.h"
#include "errors.h"
#include "ArduinoJson-v5.13.2.h"
#include "FS.h"
#include <LITTLEFS.h>

#define COMMAND_REPLY_BUFFER_SIZE 240
#define REPLY_ELEMENT_SIZE 250
#define REPLY_ERROR_SIZE 100

// Controller - takes readings and sends them to the required destination

// The ListenerConfiguration is stored in settings and specifies a listener to be assigned when the
// node starts up. All of the items are descriptive and are persisted in the setting storage.

// A ListenerConfiguration item is used to create a commandMessageListener which is assigned to the
// sensor and actually runs the behaviour. This uses function pointers and can't be persisted in case
// the location of functions change when the software is updated. The listernName maps the configuration
// onto a function that deals with that item. The functions are defined here.

struct sensorListenerConfiguration ControllerListenerDescriptions[CONTROLLER_NO_OF_LISTENERS];

void resetListenerConfiguration(sensorListenerConfiguration *item)
{
	item->sensorName[0] = 0;
	item->listenerName[0] = 0;
	item->destination[0] = 0;
	item->sendOptionMask = 0;
}

void printListenerConfiguration(sensorListenerConfiguration *item)
{
	if (item->listenerName[0] != 0)
	{
		sensor *s = findSensorByName(item->sensorName);

		if (s == NULL)
		{
			Serial.printf("Sensor %s in listener not found\n", item->sensorName);
			return;
		}

		struct sensorEventBinder *binder = findSensorEventBinderByTrigger(s, item->sendOptionMask);

		if (item->destination[0] == 0)
		{
			Serial.printf("Process:%s Command:%s Sensor:%s Trigger:%s\n",
						  item->commandProcess,
						  item->commandName,
						  item->sensorName,
						  binder->listenerName);
		}
		else
		{
			Serial.printf("Process:%s Command:%s Sensor:%s Trigger:%s Destination:%s\n",
						  item->commandProcess,
						  item->commandName,
						  item->sensorName,
						  binder->listenerName,
						  item->destination);
		}
	}
}

void iterateThroughListenerConfigurations(void (*func)(struct sensorListenerConfiguration *commandItem))
{
	for (int i = 0; i < CONTROLLER_NO_OF_LISTENERS; i++)
	{
		func(&ControllerListenerDescriptions[i]);
	}
}

struct sensorListenerConfiguration *searchThroughControllerListeners(bool (*test)(struct sensorListenerConfiguration *commandItem, void *critereon), void *criteron)
{
	for (int i = 0; i < CONTROLLER_NO_OF_LISTENERS; i++)
	{
		if (test(&ControllerListenerDescriptions[i], criteron))
		{
			return &ControllerListenerDescriptions[i];
		}
	}
	return NULL;
}

bool matchListenerConfigurationName(struct sensorListenerConfiguration *commandItem, void *critereon)
{
	char *name = (char *)critereon;

	return (strcasecmp(commandItem->listenerName, name) == 0);
}

bool matchEmptyListener(struct sensorListenerConfiguration *commandItem, void *critereon)
{
	return commandItem->listenerName[0] == 0;
}

struct sensorListenerConfiguration *findListenerByName(char *name)
{
	return searchThroughControllerListeners(matchListenerConfigurationName, name);
}

struct sensorListenerConfiguration *findEmptyListener()
{
	return searchThroughControllerListeners(matchEmptyListener, NULL);
}

void resetControllerListenersToDefaults()
{
	iterateThroughListenerConfigurations(resetListenerConfiguration);
}

void resetSensorListenersToDefaults(char *sensorName)
{
	TRACE("Resetting listeners for sensor:");
	TRACELN(sensorName);

	for (int i = 0; i < CONTROLLER_NO_OF_LISTENERS; i++)
	{
		if (strcasecmp(sensorName, ControllerListenerDescriptions[i].sensorName) == 0)
		{
			TRACE("   resetting:");
			TRACELN(ControllerListenerDescriptions[i].listenerName);
			resetListenerConfiguration(&ControllerListenerDescriptions[i]);
		}
	}
}

void printControllerListeners()
{
	iterateThroughListenerConfigurations(printListenerConfiguration);
}

void iterateThroughControllerListenerSettingCollections(void (*func)(unsigned char *settings, int size))
{
	func((unsigned char *)ControllerListenerDescriptions, sizeof(ControllerListenerDescriptions));
}

struct controllerSettings controllerSettings;

struct SettingItem controllerActive = {
	"Controller active",
	"controlleractive",
	&controllerSettings.active,
	ONOFF_INPUT_LENGTH,
	yesNo,
	setTrue,
	validateYesNo};

struct SettingItem *controllerSettingItemPointers[] =
	{
		&controllerActive};

struct SettingItemCollection controllerSettingItems = {
	"Controller",
	"System control configuration",
	controllerSettingItemPointers,
	sizeof(controllerSettingItemPointers) / sizeof(struct SettingItem *)};

struct controllerHandler
{
	const char *sensorName;
	struct sensorListener *listener;
};

struct sensorListener *messageListeners[] = {};

void iterateThroughMessageListeners(bool (*func)(sensorListener *s))
{
	for (unsigned int i = 0; i < sizeof(messageListeners) / sizeof(struct sensorListener *); i++)
	{
		func(messageListeners[i]);
	}
}

bool addMessageListener(sensorListener *m)
{
	// find the sensor that matches this controller handler

	struct sensorListenerConfiguration *config = m->config;

	sensor *s = findSensorByName(config->sensorName);

	if (s == NULL)
	{
		TRACE("    ***No sensor:");
		TRACE(config->sensorName);
		TRACE(" to match this listener:");
		TRACELN(config->listenerName);

		return false;
	}
	else
	{
		TRACE("    Added listener:");
		TRACE(config->listenerName);
		TRACE(" to sensor:");
		TRACELN(config->sensorName);

		addMessageListenerToSensor(s, m);

		return true;
	}
}

void clearAllListeners()
{
	resetControllerListenersToDefaults();

	removeAllSensorMessageListeners();

	saveSettings();
}

boolean validateControllerCommandString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, CONTROLLER_COMMAND_LENGTH));
}

boolean validateControllerOptionString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, OPTION_STRING_LENGTH));
}

boolean setDefaultEmptyString(void *dest)
{
	char *destChar = (char *)dest;
	*destChar = 0;
	return true;
}

bool noDefaultAvailable(void *dest)
{
	return false;
}

boolean setDefaultZero(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
	return true;
}

sensorListener *makeSensorListenerFromConfiguration(struct sensorListenerConfiguration *source)
{
	TRACELN("Making sensor listener from configuration..");

	sensor *sensorListener = findSensorByName(source->sensorName);

	if (sensorListener == NULL)
	{
		TRACE("  failed to find sensor:");
		TRACELN(source->sensorName);
		return NULL;
	}

	// now need to configure the mask from the sensor
	// We do this once when we make the configuration so that
	// the sensor can process messages more quickly

	struct sensorEventBinder *listener = findSensorListenerByName(sensorListener, source->listenerName);

	if (listener == NULL)
	{
		TRACE("  failed to find listener:");
		TRACE(source->listenerName);
		TRACE("  in sensor:");
		TRACELN(source->sensorName);
		return NULL;
	}

	Command *targetCommand = FindCommandByName(source->commandProcess, source->commandName);

	if (targetCommand == NULL)
	{
		TRACE("  failed to find command:");
		TRACE(source->commandName);
		TRACE("  in processs:");
		TRACELN(source->commandProcess);
		return NULL;
	}

	// getListener will get a listener from the discarded list or create a new one
	// as required

	struct sensorListener *result = getNewSensorListener();

	result->config = source;
	result->sensor = sensorListener;
	result->lastReadingMillis = 0;
	result->receiveMessage = targetCommand->performCommand;

	return result;
}

bool clearSensorListeners(sensor *s)
{
	if (s == NULL)
		return false;

	removeAllMessageListenersFromSensor(s);

	resetSensorListenersToDefaults(s->sensorName);

	saveSettings();

	return true;
}

bool clearSensorNameListeners(char *sensorName)
{
	TRACE("Clearing listeners for sensor:");
	TRACELN(sensorName);

	sensor *s = findSensorByName(sensorName);
	return clearSensorListeners(s);
}

void startSensorListener(struct sensorListenerConfiguration *commandItem)
{
	// ignore empty handlers
	if (commandItem->listenerName[0] == 0)
	{
		return;
	}

	sensorListener *newListener = makeSensorListenerFromConfiguration(commandItem);

	// add it to the sensor

	struct sensor *targetSensor = findSensorByName(commandItem->sensorName);

	addMessageListenerToSensor(targetSensor, newListener);
}

char command_reply_buffer[COMMAND_REPLY_BUFFER_SIZE];

StaticJsonBuffer<1000> jsonBuffer;

void build_command_reply(int errorNo, JsonObject &root, char *resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];
	char errorDescription[REPLY_ERROR_SIZE];

	decodeError(errorNo, errorDescription,REPLY_ERROR_SIZE);

	const char *sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		snprintf(replyBuffer,REPLY_ELEMENT_SIZE, "\"error\":%d,\"message\":\"%s\",\"seq\":%d", errorNo, errorDescription, sequenceNo);
	}
	else
	{
		snprintf(replyBuffer, REPLY_ELEMENT_SIZE, "\"error\":%d,\"message\":\"%s\"", errorNo,errorDescription);
	}
	strcat(resultBuffer, replyBuffer);
}

void build_text_value_command_reply(int errorNo, const char *result, JsonObject &root, char *resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char *sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":%s\",\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":%s,\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void abort_json_command(int error, JsonObject &root, void (*deliverResult)(char *resultText))
{
	build_command_reply(error, root, command_reply_buffer);
	// append the version number to the invalid command message
	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

void do_Json_setting(JsonObject &root, void (*deliverResult)(char *resultText))
{
	const char *setting = root["setting"];

	TRACE("Received setting: ");
	TRACELN(setting);

	SettingItem *item = findSettingByName(setting);

	if (item == NULL)
	{
		build_command_reply(JSON_MESSAGE_COMMAND_NAME_INVALID, root, command_reply_buffer);
	}
	else
	{
		char buffer[120];

		if (!root.containsKey("value"))
		{
			// no value - just a status request
			TRACELN("  No value part");
			sendSettingItemToJSONString(item, buffer, 120);
			build_text_value_command_reply(WORKED_OK, buffer, root, command_reply_buffer);
		}
		else
		{
			// got a value part
			const char *inputSource = NULL;

			if (root["value"].is<int>())
			{
				// need to convert the input value into a string
				// as our value parser uses strings as inputs
				int v = root["value"];
				TRACE("  Setting int ");
				TRACELN(v);
				snprintf(buffer, 120, "%d", v);
				inputSource = buffer;
			}
			else
			{
				if (root["value"].is<char *>())
				{
					inputSource = root["value"];
					TRACE("  Setting string ");
					TRACELN(inputSource);
				}
				else
				{
					TRACELN("  Unrecognised setting");
				}
			}

			if (inputSource == NULL)
			{
				build_command_reply(JSON_MESSAGE_INVALID_DATA_TYPE, root, command_reply_buffer);
			}
			else
			{
				if (item->validateValue(item->value, inputSource))
				{
					saveSettings();
					build_command_reply(WORKED_OK, root, command_reply_buffer);
				}
				else
				{
					build_command_reply(JSON_MESSAGE_INVALID_DATA_VALUE, root, command_reply_buffer);
				}
			}
		}
	}

	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

void appendCommandItemType(CommandItem *item, char *buffer, int bufferSize)
{
	switch (item->type)
	{
	case textCommand:
		snprintf(buffer, bufferSize, "%stext", buffer);
		break;

	case integerCommand:
		snprintf(buffer, bufferSize, "%sint", buffer);
		break;

	case floatCommand:
		snprintf(buffer, bufferSize, "%sfloat", buffer);
		break;
	}
}

void appendCommandDescriptionToJson(Command *command, char *buffer, int bufferSize)
{
	snprintf(buffer, bufferSize, "%s{\"name\":\"%s\",\"version\":\"%s\",\"desc\":\"%s\",\"items\":[",
			 buffer, command->name, Version, command->description);

	for (int i = 0; i < command->noOfItems; i++)
	{
		if (i > 0)
		{
			snprintf(buffer, bufferSize, "%s,", buffer);
		}
		CommandItem *item = command->items[i];

		snprintf(buffer, bufferSize, "%s{\"name\":\"%s\",\"optional\":%d,\"desc\":\"%s\",\"type\":\"",
				 buffer,
				 item->name,
				 item->setDefaultValue != noDefaultAvailable,
				 item->description);
		appendCommandItemType(item, buffer, bufferSize);
		snprintf(buffer, bufferSize, "%s\"}", buffer);
	}

	snprintf(buffer, bufferSize, "%s]}", buffer);
}

void appendCommandDescriptionToText(Command *command, char *buffer, int bufferSize)
{
	snprintf(buffer, bufferSize, "%s    %s - %s\n",
			 buffer, command->name, command->description);

	for (int i = 0; i < command->noOfItems; i++)
	{
		CommandItem *item = command->items[i];
		snprintf(buffer, bufferSize, "%s        %s - %s : ", buffer, item->name, item->description);
		appendCommandItemType(item, buffer, bufferSize);
		if (item->setDefaultValue != noDefaultAvailable)
		{
			snprintf(buffer, bufferSize, "%s (optional)",
					 buffer);
		}
		snprintf(buffer, bufferSize, "%s\n", buffer);
		continue;

		if (item->setDefaultValue == noDefaultAvailable)
		{
			snprintf(buffer, bufferSize, "      %s%s  - %s:",
					 buffer,
					 item->name,
					 item->description);
		}
		else
		{
			snprintf(buffer, bufferSize, "     %s%s* - %s:",
					 buffer,
					 item->name,
					 item->description);
		}
		appendCommandItemType(item, buffer, bufferSize);
		snprintf(buffer, bufferSize, "%s", buffer);
	}
}

void dumpCommand(const char *processName, const char *commandName, unsigned char *commandParameterBuffer)
{
	Command *command = FindCommandByName(processName, commandName);

	for (int i = 0; i < OPTION_STORAGE_SIZE; i++)
	{
		Serial.printf("%x:%c ", commandParameterBuffer[i], commandParameterBuffer[i]);
	}

	Serial.println("\n\n");

	if (command == NULL)
	{
		Serial.printf("Process %s not found\n", processName);
		return;
	}

	Serial.printf("Dumping command: %s with %d items\n", command->name, command->noOfItems);

	for (int i = 0; i < command->noOfItems; i++)
	{
		CommandItem *item = command->items[i];

		Serial.printf("Offset:%d ", item->commandSettingOffset);

		int intValue;
		float floatValue;

		switch (item->type)
		{
		case textCommand:
			Serial.printf("   %s: %s\n", item->name, (char *)commandParameterBuffer + item->commandSettingOffset);
			break;

		case integerCommand:
			intValue = getUnalignedInt(commandParameterBuffer + item->commandSettingOffset);
			Serial.printf("   %s: %d\n", item->name, intValue);
			break;

		case floatCommand:
			floatValue = getUnalignedFloat(commandParameterBuffer + item->commandSettingOffset);
			Serial.printf("   %s: %f\n", item->name, floatValue);
			break;
		}
	}
}

int CreateSensorListener(
	sensor *targetSensor,
	process *targetProcess,
	Command *targetCommand,
	sensorEventBinder *targetListener,
	char *destination,
	unsigned char *commandParameterBuffer)
{
	TRACE("Creating listener:");
	TRACE(targetListener->listenerName);
	TRACE(" for process:");
	TRACE(targetProcess->processName);
	TRACE(" assigned to sensor:");
	TRACE(targetSensor->sensorName);
	TRACE(" destination:");
	TRACE(destination);
	TRACE("send on change:");
	TRACELN(targetListener->trigger);

	// Make sure we have a target for this listener

	struct sensorListenerConfiguration *dest = NULL;

	for (int i = 0; i < CONTROLLER_NO_OF_LISTENERS; i++)
	{
		struct sensorListenerConfiguration *sensorConfig = &ControllerListenerDescriptions[i];

		if (strcasecmp(sensorConfig->sensorName, targetSensor->sensorName) == 0)
		{
			// got a matching target sensor
			if (strcasecmp(destination, sensorConfig->destination) == 0)
			{
				// .. and got a matching destination
				if (strcasecmp(targetListener->listenerName, sensorConfig->listenerName) == 0)
				{
					// .. and got a matching target process
					if (strcasecmp(targetProcess->processName, sensorConfig->commandProcess) == 0)
					{
						TRACELN("Found the listener");
						dest = sensorConfig;
						break;
					}
				}
			}
		}
	}

	if (dest == NULL)
	{
		// Nothing already in the table - need to make a new entry
		// find an empty listener to store this in...

		TRACELN("Finding an empty listener config slot");

		dest = findEmptyListener();

		if (dest == NULL)
		{
			return JSON_MESSAGE_NO_ROOM_TO_STORE_LISTENER;
		}

		TRACELN("Got an empty listener config slot");

		// if the listener was already present it will already be bound to the
		// sensor becuase we do this at boot
		// As this is a new listener we now need to bind it to the sensor

		// copy the incoming command into the storage at the given location

		// need to do this piecemeal..
		strcpy(dest->commandProcess, targetProcess->processName);
		strcpy(dest->commandName, targetCommand->name);
		strcpy(dest->listenerName, targetListener->listenerName);
		strcpy(dest->sensorName, targetSensor->sensorName);
		strcpy(dest->destination, destination);
		dest->sendOptionMask = targetListener->trigger;

		// copy the command options into the new listener config slot

		memcpy(dest->optionBuffer, commandParameterBuffer, OPTION_STORAGE_SIZE);

		// set the sensor option mask for this listener
		dest->sendOptionMask = targetListener->trigger;

		// make a new listener

		TRACELN("Creating a listener");

		sensorListener *newListener = makeSensorListenerFromConfiguration(dest);

		if (newListener == NULL)
		{
			TRACELN("Listener creation failed");
			return JSON_MESSAGE_LISTENER_COULD_NOT_BE_CREATED;
		}

		// add it to the sensor

		TRACELN("Adding the listener to the sensor");

		addMessageListenerToSensor(targetSensor, newListener);

		//dumpCommand(targetProcess->processName, targetCommand->name, dest->optionBuffer);

		TRACELN("All done");
	}
	else
	{
		// just copy the incoming command into the storage as the listener is already active
		memcpy(dest->optionBuffer, commandParameterBuffer, OPTION_STORAGE_SIZE);
		// set the sensor option mask for this listener
		dest->sendOptionMask = targetListener->trigger;
	}

	saveSettings();

	return WORKED_OK;
}

bool buildStoreFolderName(char *dest, int length, const char * store)
{
	if(store[0] != '/')
	{
		snprintf(dest,length,"/%s", store);
	}
	else
	{
		snprintf(dest,length,"%s", store);
	}
	return true;
}

// Builds and verifies the name of a stored file
// At the moment it just appends the strings.

bool buildStoreFilename(char *dest, int length, const char *store, const char *name)
{
	if(store[0] != '/')
	{
		snprintf(dest, length, "/%s/%s", store, name);
	}
	else 
	{
		snprintf(dest, length, "%s/%s", store, name);
	}
	return true;
}

// We can put commands into "stores" which equate to
// file folders. All the commands in a store can be triggered
// allowing us to create macros. Some stores have special names
// and contain commands that are to be performed at a particular time
// We have a boot store, a Wifi store, a Clock store and an MQTT store which
// can contain commands for those events. Starting with the boot one first.
// This function checks for a store property and puts the command in that store
// If the store (folder) does not exist it will be created

int checkAndAddToStore(JsonObject &root)
{
	TRACELN("Checking if a command should be added to a store:");

	const char *commandStoreName = root["store"];

	if (commandStoreName == NULL)
	{
		TRACELN("    No store command.");
		return WORKED_OK;
	}

	const char *commandID = root["id"];

	if (commandID == NULL)
	{
		TRACELN("    Store command missing ID.");
		return JSON_MESSAGE_STORE_ID_MISSING_FROM_STORE_COMMAND;
	}

	char fullStoreName[STORE_FILENAME_LENGTH];

	if(!buildStoreFolderName(fullStoreName,STORE_FILENAME_LENGTH, commandStoreName ))
	{
		return JSON_MESSAGE_STORE_FOLDERNAME_INVALID;
	}

	File folder = LittleFS.open(fullStoreName, "r");

	if (!folder)
	{
		TRACE("    Creating a folder for:");
		TRACELN(fullStoreName);
		// need to create a folder for this store
		bool dirMake = LittleFS.mkdir(fullStoreName);

		if (!dirMake)
		{
			TRACELN("   folder could not be created");
			return JSON_MESSAGE_COULD_NOT_CREATE_STORE_FOLDER;
		}

		folder = LittleFS.open(fullStoreName, "r");
	}

	if (!folder.isDirectory())
	{
		// this should be a directory
		TRACELN("    File found in place of folder for store");
		folder.close();
		return JSON_MESSAGE_FILE_IN_PLACE_OF_STORE_FOLDER;
	}

	char fullFileName[STORE_FILENAME_LENGTH];

	if (!buildStoreFilename(fullFileName, STORE_FILENAME_LENGTH, commandStoreName, commandID))
	{
		folder.close();
		return JSON_MESSAGE_STORE_FILENAME_INVALID;
	}

	TRACE("    storing the command in:");
	TRACELN(fullFileName);

	File outputFile = LittleFS.open(fullFileName, "w");
	
	// Remove these tags from the saved command
	root.remove("store");
	root.remove("id");

	char rawCommandText[500];

	root.printTo(rawCommandText, 500);

	TRACE("    storing the command:");
	TRACELN(rawCommandText);

	outputFile.printf("%s\n", rawCommandText);

	outputFile.close();

	return WORKED_OK;
}

int performCommandsInStore(char *commandStoreName)
{
	TRACE("Performing the commands in command store folder:");
	TRACELN(commandStoreName);

	char fullStoreName[STORE_FILENAME_LENGTH];

	if(!buildStoreFolderName(fullStoreName,STORE_FILENAME_LENGTH, commandStoreName ))
	{
		return JSON_MESSAGE_STORE_FOLDERNAME_INVALID;
	}

	File folder = LittleFS.open(fullStoreName, "r");

	if (!folder)
	{
		TRACELN("    Folder does not exist");
		return JSON_MESSAGE_STORE_FOLDER_DOES_NOT_EXIST;
	}

	while (true)
	{
		File entry = folder.openNextFile();

		if (!entry)
		{
			// no more files in the folder
			break;
		}

		TRACE("     Found file:");
		TRACELN(entry.name());

		String line = entry.readStringUntil('\n');

		const char * lineChar = line.c_str();
		TRACE("    Contains command:");
		TRACELN(lineChar);

		performRemoteCommand((char *)lineChar);

		entry.close();
	}
	return WORKED_OK;
}

unsigned char commandParameterBuffer[OPTION_STORAGE_SIZE];

int decodeCommand(const char *rawCommandText, process *process, Command *command,
				  unsigned char *parameterBuffer, JsonObject &root)
{
	TRACELN("Decoding a command");
	char buffer[120];

	char destination[DESTINATION_NAME_LENGTH];

	int failcount = 0;

	const char *sensorName = root["sensor"];

	for (int i = 0; i < command->noOfItems; i++)
	{
		CommandItem *item = command->items[i];

		const char *option = root[item->name];

		TRACE("Handling option:");
		TRACELN(item->name);

		if (option == NULL)
		{
			TRACELN("  Option not supplied in command");

			if (item->setDefaultValue(parameterBuffer + item->commandSettingOffset))
			{
				TRACE("    Found a default option - all good");
				// set OK - move on to the next one
				continue;
			}
			else
			{
				TRACE("    Not got a default option - ");
				if (strcasecmp(item->name, "value")==0 || strcasecmp(item->name, "text")==0 ){
					if(sensorName != NULL){
						TRACELN("no need for default as it is a sensor command");
						continue;
					}
				}
				TRACELN("command failed");
				TRACELN(item->name);
				return JSON_MESSAGE_COMMAND_ITEM_NOT_FOUND;
			}
		}

		const char *inputSource = NULL;

		if (root[item->name].is<int>())
		{
			TRACE("Got an int:");
			// need to convert the input value into a string
			// as our value parser uses strings as inputs
			int iv = root[item->name];
			TRACE(iv);
			TRACE(" for ");
			TRACELN(item->name);
			snprintf(buffer, 120, "%d", iv);
			inputSource = buffer;
		}
		else
		{
			if (root[item->name].is<float>())
			{
				TRACELN("Got a float:");
				// need to convert the input value into a string
				// as our value parser uses strings as inputs
				float fv = root[item->name];
				TRACE(fv);
				TRACE(" for ");
				TRACELN(item->name);
				snprintf(buffer, 120, "%f", fv);
				inputSource = buffer;
			}
			else
			{
				if (root[item->name].is<char *>())
				{
					inputSource = root[item->name];
					TRACE("Got a string:");
					TRACE(inputSource);
					TRACE(" for ");
					TRACELN(item->name);
				}
				else
				{
					TRACE("Command item invalid:");
					TRACELN(item->name);
					return JSON_MESSAGE_COMMAND_ITEM_INVALID;
				}
			}
		}

		if (inputSource == NULL)
		{
			failcount++;
		}
		else
		{
			if (!item->validateValue(parameterBuffer + item->commandSettingOffset, inputSource))
			{
				failcount++;
			}
		}
	}

	if (failcount != 0)
	{
		return JSON_MESSAGE_COMMAND_ITEM_INVALID;
	}

	// need to get the destination of this command

	const char *destSource = root["to"];

	if (destSource == NULL)
	{
		// empty destination string
		destination[0] = 0;
	}
	else
	{
		if (!validateString(destination, destSource, DESTINATION_NAME_LENGTH))
		{
			return JSON_MESSAGE_DESTINATION_STRING_TOO_LONG;
		}
	}

	// We have a valid command - see if it is being controlled by a sensor trigger

	int result = WORKED_OK;

	if (sensorName != NULL)
	{
		TRACELN("   adding a listener");
		// Creating and adding a sensor with a trigger
		// The command will not be performed now
		const char *trigger = root["trigger"];

		if (trigger == NULL)
		{
			return JSON_MESSAGE_SENSOR_MISSING_TRIGGER;
		}

		sensor *s = findSensorByName(sensorName);

		if (s == NULL)
		{
			return JSON_MESSAGE_SENSOR_ITEM_NOT_FOUND;
		}

		sensorEventBinder *binder = findSensorListenerByName(s, trigger);

		if (binder == NULL)
		{
			return JSON_MESSAGE_NO_MATCHING_SENSOR_FOR_LISTENER;
		}

		result = CreateSensorListener(s, process, command, binder, destination, commandParameterBuffer);
	}
	else
	{
		// Performing a command now
		TRACELN("   performing a command");
		result = command->performCommand(destination, parameterBuffer);
	}

	if (result == WORKED_OK)
	{
		// command performed/listener assigned successfully
		// see if it should be added to a command folder
		result = checkAndAddToStore(root);
	}

	TRACELN("Done decoding");

	return result;
}

void do_Json_command(const char *rawCommandText, JsonObject &root, void (*deliverResult)(char *resultText))
{
	TRACELN();
	TRACELN("Doing JSON command");
	const char *processName = root["process"];
	Command *command = NULL;
	struct process *process = NULL;

	int error = WORKED_OK;

	if (processName == NULL)
	{
		error = JSON_MESSAGE_PROCESS_NAME_MISSING;
	}
	else
	{
		TRACE("  for process: ");
		TRACE(processName);

		process = findProcessByName(processName);

		if (process == NULL)
		{
			TRACELN("   Process name invalid");
			error = JSON_MESSAGE_PROCESS_NAME_INVALID;
		}
		else
		{
			const char *commandName = root["command"];

			if (commandName == NULL)
			{
				TRACELN("   Process command missing command");
				error = JSON_MESSAGE_COMMAND_MISSING_COMMAND;
			}
			else
			{
				TRACE(" command: ");
				TRACELN(commandName);
				TRACE("  ");
				command = FindCommandInProcess(process, commandName);
				if (command == NULL)
				{
					error = JSON_MESSAGE_COMMAND_COMMAND_NOT_FOUND;
				}
			}
		}
	}

	if (error == WORKED_OK)
	{
		error = decodeCommand(rawCommandText, process, command, commandParameterBuffer, root);
	}

	build_command_reply(error, root, command_reply_buffer);

	strcat(command_reply_buffer, "}");

	deliverResult(command_reply_buffer);

	TRACE("Done JSON command");
}

void act_onJson_message(const char *json, void (*deliverResult)(char *resultText))
{
	TRACELN();
	TRACE("Received message:");
	TRACELN(json);

	command_reply_buffer[0] = 0;

	strcat(command_reply_buffer, "{");

	// Clear any previous elements from the buffer

	jsonBuffer.clear();

	JsonObject &root = jsonBuffer.parseObject(json);

	if (!root.success())
	{
		TRACELN("JSON could not be parsed");
		abort_json_command(JSON_MESSAGE_COULD_NOT_BE_PARSED, root, deliverResult);
		return;
	}

	const char *setting = root["setting"];

	if (setting)
	{
		do_Json_setting(root, deliverResult);
		return;
	}

	const char *command = root["command"];

	if (command)
	{
		do_Json_command(json, root, deliverResult);
		return;
	}

	TRACELN("Missing setting or command");
	abort_json_command(JSON_MESSAGE_MISSING_COMMAND_NAME, root, deliverResult);
	return;
}

void createJSONfromSettings(char *processName, struct Command *command, char *destination, unsigned char *settingBase, char *buffer, int bufferLength)
{
	TRACE("Creating json for command:");
	TRACE(command->name);
	TRACE(" from process:");
	TRACE(processName);
	TRACE(" to destination ");
	TRACE(destination);
	TRACE(" with ");
	TRACE(command->noOfItems);
	TRACELN(" items");

	sprintf(buffer, "{");

	snprintf(buffer, bufferLength, "%s \"process\":\"%s\",\"command\":\"%s\",", buffer, processName, command->name);

	for (int i = 0; i < command->noOfItems; i++)
	{
		CommandItem *item = command->items[i];

		if (i != 0)
		{
			snprintf(buffer, bufferLength, "%s,", buffer);
		}

		snprintf(buffer, bufferLength, "%s \"%s\":", buffer, item->name);

		char *textPtr;
		int val;
		float floatVal;

		switch (item->type)
		{
		case textCommand:
			textPtr = (char *)(settingBase + item->commandSettingOffset);
			snprintf(buffer, bufferLength, "%s\"%s\"", buffer, textPtr);
			break;

		case integerCommand:
			val = getUnalignedInt(settingBase + item->commandSettingOffset);
			snprintf(buffer, bufferLength, "%s%d", buffer, val);
			break;

		case floatCommand:
			floatVal = getUnalignedFloat(settingBase + item->commandSettingOffset);
			snprintf(buffer, bufferLength, "%s%f", buffer, floatVal);
			break;
		}
	}

	snprintf(buffer, bufferLength, "%s, \"from\":\"%s\"}", buffer, mqttSettings.mqttDeviceName);
	Serial.printf("Built:%s", buffer);
}

void showLocalPublishCommandResult(char *resultText)
{
	Serial.println(resultText);
}

#define CONTROLLER_FLOAT_VALUE_OFFSET 0
#define CONTROLLER_MESSAGE_OFFSET (CONTROLLER_FLOAT_VALUE_OFFSET + sizeof(float))


boolean validateControllerCommandStoreString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, COMMAND_STORE_NAME_LENGTH));
}

struct CommandItem commandStoreName = {
	"store",
	"store containing the commands to be performed",
	CONTROLLER_MESSAGE_OFFSET,
	textCommand,
	validateControllerCommandStoreString,
	noDefaultAvailable};


struct CommandItem *performCommandStoreItems[] =
	{
		&commandStoreName};

int doPerformCommandStore(char *destination, unsigned char *settingBase);

struct Command performCommandStore
{
	"perform",
		"Performs the commands in a command store",
		performCommandStoreItems,
		sizeof(performCommandStoreItems) / sizeof(struct CommandItem *),
		doPerformCommandStore
};

int doPerformCommandStore(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("controller", &performCommandStore, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	Serial.println("Performing a command");

	char *command = (char *)(settingBase + CONTROLLER_MESSAGE_OFFSET);
	return performCommandsInStore(command);
}

struct Command *controlCommandList[] = {
	&performCommandStore
	};

struct CommandItemCollection controllerCommands =
	{
		"Perform commands on the device",
		controlCommandList,
		sizeof(controlCommandList) / sizeof(struct Command *)};

void initcontroller()
{
	controllerProcess.status = CONTROLLER_STOPPED;
}

void startcontroller()
{
	controllerProcess.status = CONTROLLER_OK;

	// Look for sensors and bind a message controller to each
	// need to iterate through all the controllers and add to each sensor.
	iterateThroughListenerConfigurations(startSensorListener);

	performCommandsInStore(BOOT_FOLDER_NAME);

}

void updatecontroller()
{
}

void stopcontroller()
{
	controllerProcess.status = CONTROLLER_STOPPED;
}

bool controlerStatusOK()
{
	return controllerProcess.status == CONTROLLER_OK;
}

void controllerStatusMessage(char *buffer, int bufferLength)
{
	if (controllerProcess.status == CONTROLLER_STOPPED)
		snprintf(buffer, bufferLength, "Controller stopped");
	else
		snprintf(buffer, bufferLength, "Controller active");
}

struct process controllerProcess = {
	"controller",
	initcontroller,
	startcontroller,
	updatecontroller,
	stopcontroller,
	controlerStatusOK,
	controllerStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&controllerSettings, sizeof(controllerSettings), &controllerSettingItems,
	&controllerCommands,
	BOOT_PROCESS + ACTIVE_PROCESS,
	NULL,
	NULL,
	NULL,
	NULL, // no command options
	0	  // no command options
};
