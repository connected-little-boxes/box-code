#pragma once


#define CONTROLLER_OK 800
#define CONTROLLER_STOPPED 801

#define DEFAULT_READING_INTERVAL_SECS 10

#define MAX_MESSAGE_LENGTH 80
#define VALUE_START_POSITION 0
#define MESSAGE_START_POSITION sizeof(float)

#define CONTROLLERMESSAGE_COMMAND_LENGTH 20

struct controllerSettings 
{
    bool active;
};

extern struct controllerSettings controllerSettings;
extern struct process controllerProcess;

enum CommandItem_Type { textCommand, integerCommand, floatCommand};

#define COMMAND_PARAMETER_BUFFER_LENGTH 150

struct CommandItem 
{
    char * name;
    char * description;
    int commandSettingOffset;
    CommandItem_Type type;
    bool (*validateValue)(void * dest, const char * newValueStr);
    bool (*setDefaultValue)(void * dest);
};

struct Command
{
	const char * name;
    const char * description;
    struct CommandItem ** items;
    int noOfItems;
	int (*performCommand)(char * destination, unsigned char * commandData);
};

struct CommandItemCollection
{
    char * description;
	Command ** commands;
	int noOfCommands;
};

void act_onJson_message(const char *json, void (*deliverResult)(char *resultText));

bool setDefaultEmptyString(void * dest);
bool noDefaultAvailable(void * dest);
bool setDefaultZero(void * dest);

void resetControllerListenersToDefaults();

void printControllerListeners();

void iterateThroughControllerListenerSettingCollections(void (*func) (unsigned char * settings, int size));

#define JSON_BUFFER_SIZE 200

void createJSONfromSettings(char * processName, struct Command * command,  char * destination, unsigned char * settingBase, char * buffer, int bufferLength);

void dumpCommand(const char * processName,const char * commandName, unsigned char * commandParameterBuffer);

void appendCommandDescriptionToJson(Command * command, char * buffer, int bufferSize);
void appendCommandDescriptionToText(Command * command, char * buffer, int bufferSize);
void appendCommandItemType(CommandItem * item, char * buffer, int bufferSize);
void clearAllListeners();
