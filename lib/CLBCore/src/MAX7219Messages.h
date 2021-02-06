#pragma once

#include "controller.h"

#define MAX7219MESSAGES_OK 1200
#define MAX7219MESSAGES_STOPPED 1201

#define MAX7219_HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define MAX7219_MAX_FRAME_TIME_UPPER_LIMIT 1000
#define MAX7219_DEFAULT_MAX_FRAME_TIME 200
#define MAX7219_DEFAULT_FRAME_TIME_FRACTION 0.1
#define MAX7219_DEFAULT_BRIGHTNESS_FRACTION 0.5

#define MAX7219MESSAGE_COMMAND_LENGTH 20

struct Max7219MessagesSettings
{
    bool max7219MessagesEnabled;
    char max7219DefaultMessage [MAX_MESSAGE_LENGTH];
    int maxFrameTimeMS;
    float frameTimeFraction;
    float brightness;
    int dataPin;
    int chipSelectPin;
    int clockPin;
    int xDevices;
    int yDevices;
};

void displayMAX7219Message(char *messageText, char * options);

void max7219max7219MessagesOff();

void max7219max7219MessagesOn();

extern struct max7219MessagesSettings max7219max7219MessagesSettings;

extern struct SettingItemCollection max7219max7219MessagesSettingItems;

extern struct process max7219MessagesProcess;
