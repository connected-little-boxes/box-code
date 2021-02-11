#include "debug.h"

#include "controller.h"
#include "processes.h"
#include "console.h"
#include "mqtt.h"
#include "debug.h"
#include "utils.h"
#include "errors.h"
#include "settings.h"

#include "ArduinoJson-v5.13.2.h"

#define COMMAND_REPLY_BUFFER_SIZE 240
#define REPLY_ELEMENT_SIZE 100

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
		sensor * s = findSensorByName(item->sensorName);

		if(s==NULL){
			Serial.printf("Sensor %s in listener not found\n", item->sensorName);
			return;
		}

		struct sensorEventBinder * binder = findSensorEventBinderByMask(s, item->sendOptionMask);

		if(item->destination[0]==0)
		{
			Serial.printf("Process:%s Command:%s Sensor:%s Trigger:%s\n",
					  item->commandProcess,
					  item->commandName, 
					  item->sensorName,
					  binder->listenerName
					  );
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

void resetSensorListenersToDefaults(char * sensorName)
{
	TRACE("Resetting listeners for sensor:");
	TRACELN(sensorName);

	for (int i = 0; i < CONTROLLER_NO_OF_LISTENERS; i++)
	{
		if(strcasecmp(sensorName, ControllerListenerDescriptions[i].sensorName)==0)
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

#define CONTROLLER_OPTION_OFFSET (MESSAGE_START_POSITION + MAX_MESSAGE_LENGTH)

struct CommandItem ControllerfloatValueItem = {
    "value",
    "value",
    VALUE_START_POSITION,
    floatCommand,
    validateFloat,
    noDefaultAvailable};

struct CommandItem ControllerCommandSensorName = {
    "clearsensor",
    "sensor to act on",
    CONTROLLER_OPTION_OFFSET,
    textCommand,
    validateControllerOptionString,
    noDefaultAvailable};

struct CommandItem *ControllerClearAllListenersCommandItems[] =
    {
    };

int doControllerClearAllListeners(char *destination, unsigned char *settingBase);

struct Command ControllerClearAllListenersCommand
{
    "clearlisteners",
        "Clears all the event listeners",
        ControllerClearAllListenersCommandItems,
        sizeof(ControllerClearAllListenersCommandItems) / sizeof(struct CommandItem *),
        doControllerClearAllListeners
};

int doControllerClearAllListeners(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("controller", &ControllerClearAllListenersCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

	clearAllListeners();

    return WORKED_OK;
}

struct CommandItem *ControllerClearSensorListenersCommandItems[] =
    {
		&ControllerCommandSensorName
    };

int doControllerClearSensorListeners(char *destination, unsigned char *settingBase);

struct Command ControllerClearSensorListenersCommand
{
    "clearsensorlisteners",
        "Clears the event listeners for a sensor",
        ControllerClearSensorListenersCommandItems,
        sizeof(ControllerClearSensorListenersCommandItems) / sizeof(struct CommandItem *),
        doControllerClearSensorListeners
};

int doControllerClearSensorListeners(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("controller", &ControllerClearSensorListenersCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

	char * sensorName = (char *) settingBase + CONTROLLER_OPTION_OFFSET;

	TRACE("Clearing listeners for sensor:");
	TRACELN(sensorName);

	sensor * s = findSensorByName(sensorName);

	if(s==NULL)
	{
		TRACELN("    Sensor not found");
		return JSON_MESSAGE_SENSOR_NOT_FOUND_FOR_LISTENER_DELETE;
	}

	removeAllMessageListenersFromSensor(s);

	resetSensorListenersToDefaults(s->sensorName);

	saveSettings();

    return WORKED_OK;
}


struct Command *controllerCommandList[] = {
	&ControllerClearAllListenersCommand,
	&ControllerClearSensorListenersCommand
};

struct CommandItemCollection controllerCommands =
	{
		"Configure the message controller",
		controllerCommandList,
		sizeof(controllerCommandList) / sizeof(struct Command *)};

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

StaticJsonBuffer<600> jsonBuffer;

void build_command_reply(int errorNo, JsonObject &root, char *resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char *sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"error\":%d,\"seq\":%d", errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"error\":%d", errorNo);
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

void appendCommandItemType(CommandItem * item, char * buffer, int bufferSize)
{
	switch(item->type)
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

void appendCommandDescriptionToJson(Command * command, char * buffer, int bufferSize)
{
	snprintf(buffer, bufferSize,"%s{\"name\":\"%s\",\"version\":\"%s\",\"desc\":\"%s\",\"items\":[",
		buffer, command->name, Version, command->description);

	for(int i=0;i<command->noOfItems;i++)
	{
		if(i>0){
			snprintf(buffer, bufferSize,"%s,",buffer);
		}
		CommandItem * item = command->items[i];

		snprintf(buffer, bufferSize, "%s{\"name\":\"%s\",\"desc\":\"%s\",\"type\":\"",
		buffer,
		item->name,
		item->description);
		appendCommandItemType(item, buffer, bufferSize);
		snprintf(buffer, bufferSize, "%s\"}", buffer);
	}
	
	snprintf(buffer, bufferSize, "%s]}", buffer);
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
	TRACE(" assigned to sensor:");
	TRACE(targetSensor->sensorName);
	TRACE(" destination:");
	TRACE(destination);
	TRACE("send on change:");
	TRACELN(targetListener->optionMask);

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
					TRACELN("Found the listener");
					dest = sensorConfig;
					break;
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
		dest->sendOptionMask = targetListener->optionMask;

		// copy the command options into the new listener config slot

		memcpy(dest->optionBuffer, commandParameterBuffer, OPTION_STORAGE_SIZE);

		// set the sensor option mask for this listener
		dest->sendOptionMask = targetListener->optionMask;

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

		// dumpCommand(targetProcess->processName, targetCommand->name, dest->optionBuffer);

		TRACELN("All done");
	}
	else
	{
		// just copy the incoming command into the storage as the listener is already active
		memcpy(dest->optionBuffer, commandParameterBuffer, OPTION_STORAGE_SIZE);
		// set the sensor option mask for this listener
		dest->sendOptionMask = targetListener->optionMask;
	}

	saveSettings();

	return WORKED_OK;
}

unsigned char commandParameterBuffer[OPTION_STORAGE_SIZE];

int decodeCommand(process *process, Command *command,
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

		if (option == NULL)
		{
			// Need to treat the value item as special
			// We can live without a value item if we are using a sensor as the source
			if(sensorName != NULL)
			{
				// ignore the missing value - this will be supplied from the sensor
				continue;
			}
			
			// no option with this name - do we have a default for it?
			
			if (item->setDefaultValue(parameterBuffer + item->commandSettingOffset))
			{
				TRACE("Found a default option");
				// set OK - move on to the next one
				continue;
			}
			else
			{
				return JSON_MESSAGE_COMMAND_ITEM_NOT_FOUND;
			}
		}

		const char *inputSource = NULL;

		if (root[item->name].is<int>())
		{
			TRACELN("Got an int");
			// need to convert the input value into a string
			// as our value parser uses strings as inputs
			int iv = root[item->name];
			snprintf(buffer, 120, "%d", iv);
			inputSource = buffer;
		}
		else
		{
			if (root[item->name].is<float>())
			{
				TRACELN("Got a float");
				// need to convert the input value into a string
				// as our value parser uses strings as inputs
				float fv = root[item->name];
				snprintf(buffer, 120, "%f", fv);
				inputSource = buffer;
			}
			else
			{
				if (root[item->name].is<char *>())
				{
					inputSource = root[item->name];
				}
				else
				{
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

	const char *destSource = root["destination"];

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

	if (sensorName != NULL)
	{
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

		return CreateSensorListener(s, process, command, binder, destination, commandParameterBuffer);
	}

	return command->performCommand(destination, parameterBuffer);
}

void do_Json_command(JsonObject &root, void (*deliverResult)(char *resultText))
{
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
		TRACE("For: ");
		TRACE(processName);

		process = findProcessByName(processName);

		if (process == NULL)
		{
			error = JSON_MESSAGE_PROCESS_NAME_INVALID;
		}
		else
		{
			const char *commandName = root["command"];

			if (commandName == NULL)
			{
				error = JSON_MESSAGE_COMMAND_MISSING_COMMAND;
			}
			else
			{
				TRACE(" command: ");
				TRACE(commandName);
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
		error = decodeCommand(process, command, commandParameterBuffer, root);
	}

	build_command_reply(error, root, command_reply_buffer);

	strcat(command_reply_buffer, "}");

	deliverResult(command_reply_buffer);
}

void act_onJson_message(const char *json, void (*deliverResult)(char *resultText))
{
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
		do_Json_command(root, deliverResult);
		return;
	}

	TRACELN("Missing setting or command");
	abort_json_command(JSON_MESSAGE_MISSING_COMMAND_NAME, root, deliverResult);
	return;
}

void createJSONfromSettings(char *processName, struct Command *command, char *destination, unsigned char *settingBase, char *buffer, int bufferLength)
{
	Serial.printf("Creating json for %s to destination %s\n", command->name, destination);

	Serial.printf("Got %d command items\n", command->noOfItems);

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
	snprintf(buffer, bufferLength, "%s}", buffer);
	Serial.printf("Built:%s", buffer);
}

void showLocalPublishCommandResult(char *resultText)
{
	Serial.println(resultText);
}

bool publishCommand(char *buffer, char *destination)
{
	if (*destination == 0)
	{
		// empty destination string - perform the command locally
		act_onJson_message(buffer, showLocalPublishCommandResult);
		return true;
	}
	else
	{
		return publishBufferToMQTTTopic(buffer, destination);
	}
}

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
