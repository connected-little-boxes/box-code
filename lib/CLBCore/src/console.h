#pragma once

#include <Arduino.h>

#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "pirSensor.h"
#include "controller.h"

#define CONSOLE_COMMAND_SIZE 80
#define SERIAL_BUFFER_SIZE 1500

#define CONSOLE_MAX_MESSAGE_LENGTH 40
#define CONSOLE_PRE_MESSAGE_LENGTH 15
#define CONSOLE_POST_MESSAGE_LENGTH 15
#define CONSOLE_MAX_OPTION_LENGTH 20

#define CONSOLE_OK 500
#define CONSOLE_OFF 501

extern struct process consoleProcessDescriptor;

struct ConsoleSettings 
{
    bool echoInput;
    bool autoSaveSettings;
};

extern struct ConsoleSettings consoleSettings;

struct consoleCommand {
	char * name;
	char * commandDescription;
	void(*actOnCommand)(char * commandLine);
};


struct consoleCommand * findCommand(char * commandLine,consoleCommand * commands, int noOfCommands);
char * skipCommand(char * commandLine);
boolean performCommand(char * commandLine, consoleCommand * commands, int noOfCommands);
void performRemoteCommand(char * commandLine);

