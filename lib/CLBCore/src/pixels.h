#pragma once

#include "utils.h"
#include "settings.h"
#include "controller.h"
#include "processes.h"
#include "Colour.h"
#include "Leds.h"
#include "Frame.h"

#define PIXEL_OK 100
#define PIXEL_OFF 101
#define PIXEL_NO_PIXELS 102
#define PIXELS_STATUS_ONLY 103

#include <Adafruit_NeoPixel.h>

#define PIXEL_RING_CONFIG NEO_GRB+NEO_KHZ800
#define PIXEL_STRING_CONFIG NEO_KHZ400+NEO_RGB

#define MAX_NO_OF_PIXELS 200

#define MILLIS_BETWEEN_UPDATES 20

#define MAX_BRIGHTNESS 255

#define PIXEL_COMMAND_NAME_LENGTH 20
#define PIXEL_COLOUR_NAME_LENGTH 15

void addStatusItem(boolean status);
void beginStatusDisplay();
void renderStatusDisplay();
void setupWalkingColour(Colour colour);

void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength);

struct PixelSettings
{
	int pixelControlPinNo;
	int noOfXPixels;
	int noOfYPixels;
	int pixelConfig;
	float brightness;
};

extern Leds *leds;
extern Frame *frame;

extern struct PixelSettings pixelSettings;
extern struct colourNameLookup colourNames[];
extern int noOfColours;

extern struct SettingItemCollection pixelSettingItems;

extern struct process pixelProcess;

void fadeWalkingColour(Colour newColour, int noOfSteps);
struct colourNameLookup * findColourByName(const char * name);