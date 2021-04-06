#include "console.h"
#include "otaupdate.h"
#include "mqtt.h"
#include "errors.h"
#include "rotarySensor.h"
#include "buttonsensor.h"
#include "potSensor.h"
#include "pixels.h"
#include "connectwifi.h"
#include "settingsWebServer.h"
#include "HullOS.h"
#include "boot.h"

struct ConsoleSettings consoleSettings;

struct SettingItem echoSerialInput = {
	"Echo serial input", 
	"echoserial", 
	&consoleSettings.echoInput, 
	ONOFF_INPUT_LENGTH, 
	yesNo, 
	setTrue, 
	validateYesNo};

struct SettingItem autoSaveSettings = {
	"Auto save settings after change", 
	"autosavesettings", 
	&consoleSettings.autoSaveSettings, 
	ONOFF_INPUT_LENGTH, 
	yesNo, 
	setTrue, 
	validateYesNo};

struct SettingItem* consoleSettingItemPointers[] =
{
	&echoSerialInput,
	&autoSaveSettings
};

struct SettingItemCollection consoleSettingItems = {
	"Serial console",
	"Serial console configuration",
	consoleSettingItemPointers,
	sizeof(consoleSettingItemPointers) / sizeof(struct SettingItem*)
};

void doHelp(char * commandLine);

char * skipCommand(char * commandLine)
{
	while((*commandLine != ' ') && (*commandLine != 0))
	{
		commandLine++;		
	}

	if(*commandLine == ' ')
	{
		commandLine++;
	}

	return commandLine;
}

void doShowSettings(char * commandLine)
{
	char * filterStart = skipCommand(commandLine);

	if(*filterStart != 0)
	{
		PrintSomeSettings(filterStart);
	}
	else{
		PrintAllSettings();
	}
}

void doDumpSettings(char * commandLine)
{

	char * filterStart = skipCommand(commandLine);

	if(*filterStart != 0)
	{
		DumpSomeSettings(filterStart);
	}
	else{
		DumpAllSettings();
	}
}


#define CONSOLE_MESSAGE_SIZE 500

char consoleMessageBuffer[CONSOLE_MESSAGE_SIZE];

void doStartWebServer(char * commandLine)
{
	internalReboot(CONFIG_BOOT_NO_TIMEOUT_MODE);
}

void doDumpStatus(char * commandLine)
{
	dumpSensorStatus();
	dumpProcessStatus();
	Serial.print("Heap: ");
	Serial.println(ESP.getFreeHeap());
}

void doRestart(char * commandLine)
{
	saveSettings();
	internalReboot(DEVICE_BOOT_MODE);
}

void doClear(char * commandLine)
{
	resetSettings();
	saveSettings();
	internalReboot(DEVICE_BOOT_MODE);
}

void doSaveSettings(char * commandline)
{
	saveSettings();
	Serial.println("\nSettings saved");
}

void doTestButtonSensor(char * commandline)
{
	buttonSensorTest();
}


void doTestPIRSensor(char * commandline)
{
	pirSensorTest();
}

void doTestRotarySensor(char * commandline)
{
	rotarySensorTest();
}

void doTestPotSensor(char * commandline)
{
	potSensorTest();
}

void doDumpListeners(char * commandline)
{
	Serial.println("\nSensor Listeners\n");
	printControllerListeners();
}

void printCommandsJson(process *p) {

	if(p->commands == NULL)
	{
		return;
	}

	struct CommandItemCollection * c = p->commands;
	
	if(c->noOfCommands>0){
		// have got some commands in the collection

		Serial.printf("Process:%s commands:%s\n", p->processName, c->description);
		
		for(int i=0;i<c->noOfCommands;i++)
		{
			Command * com = c->commands[i];
			snprintf(consoleMessageBuffer, CONSOLE_MESSAGE_SIZE, "   ");
			appendCommandDescriptionToJson(com, consoleMessageBuffer, CONSOLE_MESSAGE_SIZE);
			Serial.println(consoleMessageBuffer);
		}
	}
}

void doShowRemoteCommandsJson(char * commandLine)
{
	Serial.println();
	iterateThroughAllProcesses(printCommandsJson);
}

void printCommandsText(process *p) {

	if(p->commands == NULL)
	{
		return;
	}

	struct CommandItemCollection * c = p->commands;
	
	if(c->noOfCommands>0){
		// have got some commands in the collection

		Serial.printf("Process:%s commands:%s\n", p->processName, c->description);
		
		for(int i=0;i<c->noOfCommands;i++)
		{
			Command * com = c->commands[i];
			consoleMessageBuffer[0]=0;
			appendCommandDescriptionToText(com, consoleMessageBuffer, CONSOLE_MESSAGE_SIZE);
			Serial.println(consoleMessageBuffer);
		}
	}
}

void doShowRemoteCommandsText(char * commandLine)
{
	Serial.println();
	iterateThroughAllProcesses(printCommandsText);
}

void appendSensorDescriptionToJson(sensor * s, char * buffer, int bufferSize)
{
	snprintf(buffer, bufferSize,"%s{\"name\":\"%s\",\"version\":\"%s\",\"triggers\":[",
		buffer, s->sensorName, Version);

	for(int i=0;i<s->noOfSensorListenerFunctions;i++)
	{
		if(i>0){
			snprintf(buffer, bufferSize,"%s,",buffer);
		}

		sensorEventBinder * binder = &s->sensorListenerFunctions[i];
		snprintf(buffer, bufferSize, "%s{\"name\":\"%s\"}",buffer,
		binder->listenerName);
	}
	
	snprintf(buffer, bufferSize, "%s]}", buffer);
}

void printSensorTriggersJson(sensor * s)
{
	if(s->noOfSensorListenerFunctions==0)
		return;

	snprintf(consoleMessageBuffer, CONSOLE_MESSAGE_SIZE, "Sensor: %s\n     ", s->sensorName);
	appendSensorDescriptionToJson(s, consoleMessageBuffer, CONSOLE_MESSAGE_SIZE);
	Serial.printf("%s\n", consoleMessageBuffer);
}

void doShowSensorsJson(char * commandLine)
{
	Serial.println();
	iterateThroughSensors(printSensorTriggersJson);
}

void appendSensorDescriptionToText(sensor * s, char * buffer, int bufferSize)
{
	snprintf(buffer, bufferSize,"%sSensor name %s\n",
		buffer, s->sensorName);

	for(int i=0;i<s->noOfSensorListenerFunctions;i++)
	{
		sensorEventBinder * binder = &s->sensorListenerFunctions[i];
		snprintf(buffer, bufferSize, "%s   trigger:%s\n",buffer,
		binder->listenerName);
	}
}

void printSensorTriggersText(sensor * s)
{
	if(s->noOfSensorListenerFunctions==0)
		return;

	consoleMessageBuffer[0]=0;

	appendSensorDescriptionToText(s, consoleMessageBuffer, CONSOLE_MESSAGE_SIZE);
	Serial.printf("%s\n", consoleMessageBuffer);
}

void doShowSensorsText(char * commandLine)
{
	Serial.println();
	iterateThroughSensors(printSensorTriggersText);
}

void act_onJson_message(const char *json, void (*deliverResult)(char *resultText));

void showRemoteCommandResult(char * resultText)
{
	Serial.println(resultText);
}

void performRemoteCommand(char * commandLine)
{
	act_onJson_message(commandLine,showRemoteCommandResult);
}

void doClearAllListeners(char * commandLine)
{
	Serial.println("\nClearing all the listeners\n");
	clearAllListeners();
}

void doClearSensorListeners(char * commandLine)
{
	char * sensorName = skipCommand(commandLine);

	if(clearSensorNameListeners(sensorName))
	{
		Serial.println("Sensor listeners cleared");
	}
	else
	{
		Serial.println("Sensor not found");
	}
}

void doDumpStorage(char * commandLine)
{
	PrintStorage();
}

#ifdef OTA_UPDATE_ON

void doOTAUpdate(char * commandLine)
{
	performOTAUpdate();
}

#endif

#define ESC_KEY 0x1b

void doColourDisplay(char * commandLine)
{
	Serial.println("Press space to step through each colour");
	Serial.println("Press the ESC key to exit");
	
	for(int i=0; i<noOfColours; i++)
	{
		while(Serial.available()){
			int ch = Serial.read();
			if(ch==ESC_KEY){
				Serial.println("Colour display ended");
				return;
			}
		}
		Serial.printf("Colour:%s\n", colourNames[i].name);
		frame->fadeToColour(colourNames[i].col, 5);
		do{
			pixelProcess.udpateProcess();
			delay(20);
		} while(Serial.available()==0);
	}
	Serial.println("Colour display finished");
}


void doHullOSHelp(char * commandLine)
{
	Serial.println("Hullos help");

}

void doHullOSRun(char * commandLine)
{
	Serial.println("Hullos run");

}

struct consoleCommand HullOSCommands[] =
{
	{"help", "show all the commands", doHullOSHelp},
	{"run", "run the HullOS program ", doHullOSRun}
};

void doHullOS(char * commandLine)
{
	char * hullosCommand = skipCommand(commandLine);

	performCommand(hullosCommand, HullOSCommands, sizeof(HullOSCommands) / sizeof(struct consoleCommand));
}

void doDumpSprites(char * commandLine)
{
	frame->dump();
}

struct consoleCommand userCommands[] =
{
	{"help", "show all the commands", doHelp},
	{"host", "start the configuration web host", doStartWebServer},
	{"settings", "show all the setting values", doShowSettings},
	{"dump", "dump all the setting values", doDumpSettings},
	{"commands", "show all the remote commands", doShowRemoteCommandsText},
	{"commandsjson", "show all the remote commands in json", doShowRemoteCommandsJson},
	{"save", "save all the setting values", doSaveSettings},
	{"sensors","list all the sensor triggers", doShowSensorsText},
	{"sensorsjson","list all the sensor triggers in json", doShowSensorsJson},
	{"status", "show the sensor status", doDumpStatus},
	{"storage", "show the storage use of sensors and processes", doDumpStorage},
	{"buttontest", "test the button sensor", doTestButtonSensor},
	{"pirtest", "test the PIR sensor", doTestPIRSensor},
	{"rotarytest", "test the rotary sensor", doTestRotarySensor},
	{"pottest", "test the pot sensor", doTestPotSensor},
	{"colours", "step through all the colours", doColourDisplay},
	{"listeners", "list the command listeners", doDumpListeners},
	{"clearalllisteners", "clear all the command listeners", doClearAllListeners},
	{"clearsensorlisteners", "clear the command listeners for a sensor", doClearSensorListeners},
	{"restart", "restart the device", doRestart},
	{"hullos", "HullOS commands", doHullOS},
	{"sprites", "dump sprite data", doDumpSprites},
#ifdef OTA_UPDATE_ON
	{"otaupdate", "start an over-the-air firmware update", doOTAUpdate},
#endif
	{"clear", "clear all seeings and restart the device", doClear}
};

void doHelp(char * commandLine)
{
	Serial.printf("\n\nConnected Little Boxes\n Device Version %s\n\nThese are all the available commands.\n\n",
		Version);

	int noOfCommands = sizeof(userCommands) / sizeof(struct consoleCommand);

	for (int i = 0; i < noOfCommands; i++)
	{
		Serial.printf("    %s - %s\n", userCommands[i].name, userCommands[i].commandDescription);
	}

	Serial.printf("\nYou can view the value of any setting just by typing the setting name, for example:\n\n"
		"    mqttdevicename\n\n"
		"- would show you the MQTT device name.\n"
		"You can assign a new value to a setting, for example:\n\n"
		"     mqttdevicename=Rob\n\n"
		"- would set the name of the mqttdevicename to Rob.\n\n"
		"To see a list of all the setting names use the command settings.\n"
		"This displays all the settings, their values and names.\n"
		"To see a dump of settings (which can be restored to the device later) use dump.\n"
		"The dump and settings can be followed by a filter string to match setting names\n\n"
        "   dump pix\n\n"
        "- would dump all the settings that contain the string pix\n\n"
		"If you enter a JSON string this will be interpreted as a remote command.\n"
		"See the remote command documentation for more details of this.\n"
	);
}

boolean findCommandName(consoleCommand * com, char * name)
{
	int commandNameLength = strlen(com->name);

	for (int i = 0; i < commandNameLength; i++)
	{
		if (tolower(name[i]) != tolower(com->name[i]))
			return false;
	}

	// reached the end of the name, character that follows should be zero (end of the string)
	// or a space delimiter to the next part of the command

	if (name[commandNameLength] == 0)
		return true;

	if (name[commandNameLength] == ' ')
		return true;

	return false;
}

struct consoleCommand * findCommand(char * commandLine,consoleCommand * commands, int noOfCommands)
{
	for (int i = 0; i < noOfCommands; i++)
	{
		if (findCommandName(&commands[i], commandLine))
		{
			return &commands[i];
		}
	}
	return NULL;
}

boolean performCommand(char * commandLine, consoleCommand * commands, int noOfCommands)
{
	Serial.printf("Processing: %s ", commandLine);

	if(commandLine[0]=='{')
	{
		// treat the command as JSON
		performRemoteCommand(commandLine);
		return true;
	}

	// Look for a command with that name

	consoleCommand * comm = findCommand(commandLine, commands, noOfCommands);

	if (comm != NULL)
	{
		comm->actOnCommand(commandLine);
		return true;
	}

	// Look for a setting with that name

	processSettingCommandResult result;

	result = processSettingCommand(commandLine);

	switch (result)
	{
	case displayedOK:
		return true;
	case setOK:
		Serial.println("value set successfully");
		if(consoleSettings.autoSaveSettings)
		{
			saveSettings();
		}
		return true;
	case settingNotFound:
		Serial.println("Command/setting not found");
		return false;
	case settingValueInvalid:
		Serial.println("Setting value invalid");
		return false;
	}

	return false;
}

void showHelp()
{
	doHelp("help");
}

#define SERIAL_BUFFER_LIMIT SERIAL_BUFFER_SIZE-1

char serialReceiveBuffer[SERIAL_BUFFER_SIZE];

int serialReceiveBufferPos = 0;

void reset_serial_buffer()
{
	serialReceiveBufferPos = 0;
}

void actOnSerialCommand()
{
	performCommand(serialReceiveBuffer,userCommands,sizeof(userCommands) / sizeof(struct consoleCommand));
}

#define BACKSPACE_CHAR 0x08

void bufferSerialChar(char ch)
{
	if(consoleSettings.echoInput){
		Serial.print(ch);
		if(ch == BACKSPACE_CHAR){
			if(serialReceiveBufferPos>0)
			{
				serialReceiveBufferPos--;
				Serial.print(' ');
				Serial.print(ch);
			}
			return;
		}
	}

	if (ch == '\n' || ch == '\r' || ch == 0)
	{
		if (serialReceiveBufferPos > 0)
		{
			serialReceiveBuffer[serialReceiveBufferPos] = 0;
			actOnSerialCommand();
			reset_serial_buffer();
		}
		return;
	}

	if (serialReceiveBufferPos < SERIAL_BUFFER_SIZE)
	{
		serialReceiveBuffer[serialReceiveBufferPos] = ch;
		serialReceiveBufferPos++;
	}
}

void checkSerialBuffer()
{
	while (Serial.available())
	{
		bufferSerialChar(Serial.read());
	}
}

void initConsole()
{
	consoleProcessDescriptor.status = CONSOLE_OFF;
}


void startConsole()
{
	consoleProcessDescriptor.status = CONSOLE_OK;
}

void updateConsole()
{
	if (consoleProcessDescriptor.status == CONSOLE_OK)
	{
		checkSerialBuffer();
	}
}

void stopConsole()
{
	consoleProcessDescriptor.status = CONSOLE_OFF;
}

bool consoleStatusOK()
{
	return consoleProcessDescriptor.status == CONSOLE_OK;
}

void consoleStatusMessage(char * buffer, int bufferLength)
{
	switch (consoleProcessDescriptor.status)
	{
	case CONSOLE_OK:
		snprintf(buffer, bufferLength, "Console OK");
		break;
	case CONSOLE_OFF:
		snprintf(buffer, bufferLength, "Console OFF");
		break;
	default:
		snprintf(buffer, bufferLength, "Console status invalid");
		break;
	}
}

boolean validateConsoleCommandString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, CONSOLE_COMMAND_SIZE));
}

#define COMMANDNAME_CONSOLE_COMMAND_OFFSET 0

struct CommandItem consoleCommandName = {
	"commandtext",	
	"console command text",
	COMMANDNAME_CONSOLE_COMMAND_OFFSET,
	textCommand,
	validateConsoleCommandString,
	noDefaultAvailable
};

struct CommandItem * consoleCommandItems [] =
{
	&consoleCommandName
};

int doRemoteConsoleCommand(char * destination, unsigned char * settingBase);

struct Command performConsoleCommnad
{
	"remote",
	"Perform a remote console command",
	consoleCommandItems,
	sizeof(consoleCommandItems)/sizeof(struct CommandItem *),
	doRemoteConsoleCommand
};

int doRemoteConsoleCommand(char * destination, unsigned char * settingBase)
{
	if(*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer [JSON_BUFFER_SIZE];
		createJSONfromSettings("console",&performConsoleCommnad,destination, settingBase, buffer,JSON_BUFFER_SIZE);
		return publishBufferToMQTTTopic(buffer, destination);
	}

	char * command = (char *) (settingBase + COMMANDNAME_CONSOLE_COMMAND_OFFSET);

	if(performCommand(command,userCommands,sizeof(userCommands) / sizeof(struct consoleCommand))){
		return WORKED_OK;
	}

	return JSON_MESSAGE_INVALID_CONSOLE_COMMAND;
}

struct Command * consoleCommandList [] = {
	&performConsoleCommnad
};


struct CommandItemCollection consoleCommands =
{
	"Perform console commands on a remote device",
	consoleCommandList,
	sizeof(consoleCommandList)/sizeof(struct Command *)
};

struct process consoleProcessDescriptor = { 
	"console", 
	initConsole,
	startConsole, 
	updateConsole, 
	stopConsole, 
	consoleStatusOK,
	consoleStatusMessage, 
	false, 
	0, 
	0, 
	0,
	NULL,
	(unsigned char*) &consoleSettings, sizeof(consoleSettings), &consoleSettingItems,
	&consoleCommands,
	BOOT_PROCESS+ACTIVE_PROCESS+CONFIG_PROCESS+WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL};

