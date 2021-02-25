#pragma once

#include "utils.h"
#include "settings.h"
#include "controller.h"
#include "processes.h"
#include "colour.h"

#define PIXEL_OK 100
#define PIXEL_OFF 101
#define PIXEL_ERROR_NO_PIXELS 102

#include <Adafruit_NeoPixel.h>

#define PIXEL_RING_CONFIG NEO_GRB+NEO_KHZ800
#define PIXEL_STRING_CONFIG NEO_KHZ400+NEO_RGB

#define MAX_NO_OF_SPRITES 20

#define MAX_NO_OF_PIXELS 200

#define MILLIS_BETWEEN_UPDATES 50

#define MAX_BRIGHTNESS 255

#define PIXEL_COMMAND_NAME_LENGTH 20
#define PIXEL_COLOUR_NAME_LENGTH 15

boolean coloursEqual(ColourValue a, ColourValue b);

void beginStatusDisplay();
boolean addStatusItem(boolean status);
void renderStatusDisplay();
void setupWalkingColour(Colour colour);
void changeWalkingColour(ColourValue colour);

void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength);
boolean addStatusItem(boolean status);

struct PixelSettings
{
	int pixelControlPinNo;
	int noOfXPixels;
	int noOfYPixels;
	int noOfSprites;
	int pixelConfig;
};

extern struct PixelSettings pixelSettings;
extern struct colourLookup colourNames[];
extern int noOfColours;

extern struct SettingItemCollection pixelSettingItems;

extern struct process pixelProcess;

boolean validateColour(void* dest, const char* newValueStr);

void fadeWalkingColour(ColourValue newColour, int noOfSteps);
struct colourLookup * findColourByName(const char * name);