#pragma once

#include <Arduino.h>

#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "pirSensor.h"
#include "commands.h"

#define CONSOLE_COMMAND_SIZE 30
#define SERIAL_BUFFER_SIZE 1500

#define CONSOLE_OK 500
#define CONSOLE_OFF 501

extern struct process consoleProcessDescriptor;

struct ConsoleSettings 
{
    bool echoInput;
    bool autoSaveSettings;
};

extern struct ConsoleSettings consoleSettings;
