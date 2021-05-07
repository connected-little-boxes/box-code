
#include "pixels.h"
#include "errors.h"
#include "mqtt.h"
#include "controller.h"
#include "clock.h"

#include "Colour.h"
#include "Frame.h"
#include "Led.h"
#include "Leds.h"
#include "Sprite.h"
#include "boot.h"

// Some of the colours have been commented out because they don't render well
// on NeoPixels

Adafruit_NeoPixel *strip = NULL;

struct PixelSettings pixelSettings;

Leds *leds;
Frame *frame;

void setDefaultPixelControlPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem pixelControlPinSetting = {"Pixel Control Pin",
											 "pixelcontrolpin",
											 &pixelSettings.pixelControlPinNo,
											 NUMBER_INPUT_LENGTH,
											 integerValue,
											 setDefaultPixelControlPinNo,
											 validateInt};

void setDefaultNoOfXPixels(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 12;
}

void setDefaultNoOfYPixels(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 1;
}

struct SettingItem pixelNoOfXPixelsSetting = {"Number of X pixels (0 for pixels not fitted)",
											  "noofxpixels",
											  &pixelSettings.noOfXPixels,
											  NUMBER_INPUT_LENGTH,
											  integerValue,
											  setDefaultNoOfXPixels,
											  validateInt};

struct SettingItem pixelNoOfYPixelsSetting = {"Number of Y pixels (0 for pixels not fitted)",
											  "noofypixels",
											  &pixelSettings.noOfYPixels,
											  NUMBER_INPUT_LENGTH,
											  integerValue,
											  setDefaultNoOfYPixels,
											  validateInt};

void setDefaultPixelBrightness(void *dest)
{
	float *destFloat = (float *)dest;
	*destFloat = 1;
}

boolean validatePixelBrightness(void *dest, const char *newValueStr)
{
	float value;

	if (!validateFloat0to1(&value, newValueStr))
	{
		return false;
	}

	*(float *)dest = value;

	if (frame != NULL)
	{
		frame->fadeToBrightness(value, 10);
	}

	return true;
}

struct SettingItem pixelBrightnessSetting = {"Default pixel brightness",
											 "pixelbrightness",
											 &pixelSettings.brightness,
											 NUMBER_INPUT_LENGTH,
											 floatValue,
											 setDefaultPixelBrightness,
											 validatePixelBrightness};

void setDefaultPixelConfig(void *dest)
{
	int *destConfig = (int *)dest;
	*destConfig = 1;
}

boolean validatePixelConfig(void *dest, const char *newValueStr)
{
	int config;

	bool validConfig = validateInt(&config, newValueStr);

	if (!validConfig)
		return false;

	if (config < 1 || config > 2)
		return false;

	int *intDest = (int *)dest;

	*intDest = config;

	return true;
}

struct SettingItem pixelPixelConfig = {"Pixel config(1=ring 2=strand)",
									   "pixelconfig",
									   &pixelSettings.pixelConfig,
									   NUMBER_INPUT_LENGTH,
									   integerValue,
									   setDefaultPixelConfig,
									   validatePixelConfig};

struct SettingItem *pixelSettingItemPointers[] =
	{
		&pixelControlPinSetting,
		&pixelNoOfXPixelsSetting,
		&pixelNoOfYPixelsSetting,
		&pixelPixelConfig,
		&pixelBrightnessSetting};

struct SettingItemCollection pixelSettingItems = {
	"pixel",
	"Pixel hardware and display properties",
	pixelSettingItemPointers,
	sizeof(pixelSettingItemPointers) / sizeof(struct SettingItem *)};

unsigned long millisOfLastPixelUpdate;

void startPixelStrip()
{
	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
	{
		return;
	}

	switch (pixelSettings.pixelConfig)
	{
	case 1:
		strip = new Adafruit_NeoPixel(noOfPixels, pixelSettings.pixelControlPinNo,
									  NEO_GRB + NEO_KHZ800);
		break;
	case 2:
		strip = new Adafruit_NeoPixel(noOfPixels, pixelSettings.pixelControlPinNo,
									  NEO_KHZ400 + NEO_RGB);
		break;
	default:
		strip = NULL;
	}

	if (strip == NULL)
	{
		Serial.println("********* Pixel setup invalid");
		return;
	}

	strip->begin();
}

/// commands

boolean validatePixelCommandString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, PIXEL_COMMAND_NAME_LENGTH));
}

// The first element of a process command block is always a floating point value called "value"
// This will be loaded with a normalised value to be used by the process when it runs

#define FLOAT_VALUE_OFFSET 0
#define RED_PIXEL_COMMAND_OFFSET (FLOAT_VALUE_OFFSET + sizeof(float))
#define BLUE_PIXEL_COMMAND_OFFSET (RED_PIXEL_COMMAND_OFFSET + sizeof(float))
#define GREEN_PIXEL_COMMAND_OFFSET (BLUE_PIXEL_COMMAND_OFFSET + sizeof(float))
#define SPEED_PIXEL_COMMAND_OFFSET (GREEN_PIXEL_COMMAND_OFFSET + sizeof(float))
#define COMMAND_PIXEL_COMMAND_OFFSET (SPEED_PIXEL_COMMAND_OFFSET + sizeof(int))
#define COLOURNAME_PIXEL_COMMAND_OFFSET (COMMAND_PIXEL_COMMAND_OFFSET + PIXEL_COMMAND_NAME_LENGTH)
#define COMMAND_PIXEL_OPTION_OFFSET (COLOURNAME_PIXEL_COMMAND_OFFSET + PIXEL_COMMAND_NAME_LENGTH)
#define COMMAND_PIXEL_PATTERN_OFFSET (COMMAND_PIXEL_OPTION_OFFSET + PIXEL_COMMAND_NAME_LENGTH)

struct CommandItem redCommandItem = {
	"red",
	"amount of red (0-1)",
	RED_PIXEL_COMMAND_OFFSET,
	floatCommand,
	validateFloat0to1,
	noDefaultAvailable};

struct CommandItem blueCommandItem = {
	"blue",
	"amount of blue (0-1)",
	BLUE_PIXEL_COMMAND_OFFSET,
	floatCommand,
	validateFloat0to1,
	noDefaultAvailable};

struct CommandItem greenCommandItem = {
	"green",
	"amount of green (0-1)",
	GREEN_PIXEL_COMMAND_OFFSET,
	floatCommand,
	validateFloat0to1,
	noDefaultAvailable};

boolean setDefaultPixelChangeSteps(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 20;
	return true;
}

struct CommandItem pixelChangeStepsCommandItem = {
	"steps",
	"no of 50Hz steps to complete the change",
	SPEED_PIXEL_COMMAND_OFFSET,
	integerCommand,
	validateInt,
	setDefaultPixelChangeSteps};

struct CommandItem pixelCommandName = {
	"pixelCommand",
	"pixel settting command",
	COMMAND_PIXEL_COMMAND_OFFSET,
	textCommand,
	validatePixelCommandString,
	noDefaultAvailable};

struct CommandItem colourCommandName = {
	"colourname",
	"name of the colour to set",
	COLOURNAME_PIXEL_COMMAND_OFFSET,
	textCommand,
	validatePixelCommandString,
	noDefaultAvailable};

struct CommandItem colourCommandMask = {
	"colourmask",
	"mask of colour characters",
	COLOURNAME_PIXEL_COMMAND_OFFSET,
	textCommand,
	validatePixelCommandString,
	noDefaultAvailable};

struct CommandItem colourCommandOptionItem = {
	"options",
	"options for the colour command(timed)",
	COMMAND_PIXEL_OPTION_OFFSET,
	textCommand,
	validatePixelCommandString,
	setDefaultEmptyString};

struct CommandItem floatValueItem = {
	"value",
	"value (0-1)",
	FLOAT_VALUE_OFFSET,
	floatCommand,
	validateFloat0to1,
	noDefaultAvailable};

char *pixelDisplaySelections[] = {"walking", "mask"};

boolean validateSelectionCommandString(void *dest, const char *newValueStr)
{
	char buffer[PIXEL_COMMAND_NAME_LENGTH];

	bool commandOK = validateString(buffer, newValueStr, PIXEL_COMMAND_NAME_LENGTH);

	if (!commandOK)
		return false;

	commandOK = false;

	for (int i = 0; i < sizeof(pixelDisplaySelections) / sizeof(char *); i++)
	{
		if (strcasecmp(buffer, pixelDisplaySelections[i]) == 0)
		{
			commandOK = true;
		}
	}

	if (commandOK)
	{
		strcpy((char *)dest, buffer);
	}

	return commandOK;
}

struct CommandItem pixelPatternSelectionItem = {
	"pattern",
	"chosen pattern (walking,mask)",
	COMMAND_PIXEL_PATTERN_OFFSET,
	textCommand,
	validateSelectionCommandString,
	noDefaultAvailable};

struct CommandItem *setColourItems[] =
	{
		&redCommandItem,
		&blueCommandItem,
		&greenCommandItem,
		&pixelChangeStepsCommandItem};

int doSetPixelColor(char *destination, unsigned char *settingBase);

struct Command setPixelColourCommand
{
	"setcolour",
		"Sets the colour of the pixels",
		setColourItems,
		sizeof(setColourItems) / sizeof(struct CommandItem *),
		doSetPixelColor
};

int doSetPixelColor(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelColourCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	// sets the pixel colour - caller has set the r,g and b values

	float red = (float)getUnalignedFloat(settingBase + RED_PIXEL_COMMAND_OFFSET);
	float blue = (float)getUnalignedFloat(settingBase + BLUE_PIXEL_COMMAND_OFFSET);
	float green = (float)getUnalignedFloat(settingBase + GREEN_PIXEL_COMMAND_OFFSET);
	int steps = (int)getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	frame->fadeToColour({red, green, blue}, steps);

	return WORKED_OK;
}

struct CommandItem *setNamedPixelColourItems[] =
	{
		&colourCommandName,
		&pixelChangeStepsCommandItem};

int doSetNamedColour(char *destination, unsigned char *settingBase);

struct Command setPixelsToNamedColour
{
	"setnamedcolour",
		"Sets the pixels to a named colour",
		setNamedPixelColourItems,
		sizeof(setNamedPixelColourItems) / sizeof(struct CommandItem *),
		doSetNamedColour
};

int doSetNamedColour(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelsToNamedColour, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	struct colourNameLookup *col;

	char *colourName = (char *)(settingBase + COLOURNAME_PIXEL_COMMAND_OFFSET);

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	col = findColourByName(colourName);

	if (col != NULL)
	{
		frame->fadeToColour(col->col, steps);
		return WORKED_OK;
	}
	else
	{
		return JSON_MESSAGE_INVALID_COLOUR_NAME;
	}
}

struct CommandItem *setRandomPixelColourItems[] =
	{
		&pixelChangeStepsCommandItem,
		&colourCommandOptionItem};

int doSetRandomColour(char *destination, unsigned char *settingBase);

struct Command setPixelsToRandomColour
{
	"setrandomcolour",
		"Sets all the pixels to a random colour",
		setRandomPixelColourItems,
		sizeof(setRandomPixelColourItems) / sizeof(struct CommandItem *),
		doSetRandomColour
};

void seedRandomFromClock()
{
	if (clockSensor.status != SENSOR_OK)
	{
		// no clock - just pick a random colour
		return;
	}

	// get a seed for the random number generator
	struct clockReading *clockActiveReading;

	clockActiveReading =
		(struct clockReading *)clockSensor.activeReading;

	unsigned long seedValue = (clockActiveReading->hour * 60) + clockActiveReading->minute;

	// seed the random number generator with this value - it will only be for one number
	// but it shold ensure that we get a random range each time

	randomSeed(seedValue);
}

int doSetRandomColour(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelsToRandomColour, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	char *option = (char *)(settingBase + COMMAND_PIXEL_OPTION_OFFSET);

	if (strcasecmp(option, "timed") == 0)
	{
		seedRandomFromClock();
	}

	struct colourNameLookup *randomColour = findRandomColour();

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	frame->fadeToColour(randomColour->col, steps);

	return WORKED_OK;
}

struct CommandItem *setPixelTwinkleItems[] =
	{
		&pixelChangeStepsCommandItem,
		&colourCommandOptionItem};

int doSetTwinkle(char *destination, unsigned char *settingBase);

struct Command setPixelsToTwinkle
{
	"twinkle",
		"Sets the pixels to twinkle random colours",
		setPixelTwinkleItems,
		sizeof(setPixelTwinkleItems) / sizeof(struct CommandItem *),
		doSetTwinkle
};

int doSetTwinkle(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelsToTwinkle, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	char *option = (char *)(settingBase + COMMAND_PIXEL_OPTION_OFFSET);

	if (strcasecmp(option, "timed") == 0)
	{
		seedRandomFromClock();
	}

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	frame->fadeSpritesToTwinkle(steps);

	return WORKED_OK;
}
  
struct CommandItem *setPixelBrightnessItems[] =
	{
		&floatValueItem,
		&pixelChangeStepsCommandItem};

int doSetBrightness(char *destination, unsigned char *settingBase);

struct Command setPixelBrightness
{
	"brightness",
		"Sets the pixel brightness",
		setPixelBrightnessItems,
		sizeof(setPixelBrightnessItems) / sizeof(struct CommandItem *),
		doSetBrightness
};

int doSetBrightness(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelBrightness, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	float brightness = getUnalignedFloat(settingBase + FLOAT_VALUE_OFFSET);

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	frame->fadeToBrightness(brightness, steps);

	return WORKED_OK;
}

struct CommandItem *setPixelPatternItems[] =
	{
		&pixelPatternSelectionItem,
		&pixelChangeStepsCommandItem,
		&colourCommandMask};

int doSetPattern(char *destination, unsigned char *settingBase);

struct Command setPixelPattern
{
	"pattern",
		"Sets the pixel pattern",
		setPixelPatternItems,
		sizeof(setPixelPatternItems) / sizeof(struct CommandItem *),
		doSetPattern
};

int doSetPattern(char *destination, unsigned char *settingBase)
{
	if (*destination != 0)
	{
		// we have a destination for the command. Build the string
		char buffer[JSON_BUFFER_SIZE];
		createJSONfromSettings("pixels", &setPixelPattern, destination, settingBase, buffer, JSON_BUFFER_SIZE);
		return publishCommandToRemoteDevice(buffer, destination);
	}

	char *colourMask = (char *)(settingBase + COLOURNAME_PIXEL_COMMAND_OFFSET);
	char *pattern = (char *)(settingBase + COMMAND_PIXEL_PATTERN_OFFSET);
	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	TRACE("Got new pattern:");
	TRACE(pattern);

	if (strcasecmp(pattern, "walking") == 0)
	{
		frame->fadeSpritesToWalkingColours(colourMask, steps);
	}

	if (strcasecmp(pattern, "mask") == 0)
	{
		frame->fadeSpritesToColourCharMask(colourMask, steps);
	}
	return WORKED_OK;
}

struct Command *pixelCommandList[] = {
	&setPixelColourCommand,
	&setPixelsToNamedColour,
	&setPixelsToRandomColour,
	&setPixelsToTwinkle,
	&setPixelBrightness,
	&setPixelPattern};

struct CommandItemCollection pixelCommands =
	{
		"Control the pixels on the device",
		pixelCommandList,
		sizeof(pixelCommandList) / sizeof(struct Command *)};

void show()
{
	strip->show();
}

int *rasterLookup;

void setPixel(int no, float r, float g, float b)
{
	unsigned char rs = (unsigned char)round(r * 255);
	unsigned char gs = (unsigned char)round(g * 255);
	unsigned char bs = (unsigned char)round(b * 255);
	if (rs > 255 || gs > 255 || bs > 255)
		Serial.printf("setPixel no:%d raster:%d r:%f g:%f b:%f\n", no, rasterLookup[no], r, g, b);
	strip->setPixelColor(rasterLookup[no], rs, gs, bs);
}

// status display doesn't use the animated leds
// this means that it can overlay the display

int statusPixelNo = 0;

#define STATUS_DISPLAY_BACKGROUND \
	{                             \
		0.01, 0.01, 0.01          \
	}

void resetStatusDisplay()
{
	Colour col = STATUS_DISPLAY_BACKGROUND;

	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
		return;

	statusPixelNo = 0;

	for (int i = 0; i < noOfPixels; i++)
	{
		setPixel(i, col.Red, col.Green, col.Blue);
	}
}

void beginStatusDisplay()
{
	resetStatusDisplay();
	renderStatusDisplay();
}

boolean setStatusDisplayPixel(int pixelNumber, boolean statusOK)
{
	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
		return false;

	if (pixelNumber >= noOfPixels)
		return false;

	if (statusOK)
	{
		setPixel(pixelNumber, 0, 0.5, 0);
	}
	else
	{
		setPixel(pixelNumber, 0.5, 0, 0);
	}

	return true;
}

void renderStatusDisplay()
{
	if (strip == NULL)
		return;

	strip->show();
	delay(200);
}

void addStatusItem(boolean status)
{
	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
		return;

	if (statusPixelNo >= noOfPixels)
		resetStatusDisplay();
	setStatusDisplayPixel(statusPixelNo, status);

	statusPixelNo++;

	return;
}

void initPixel()
{
	pixelProcess.status = PIXEL_OFF;

	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
	{
		pixelProcess.status = PIXEL_NO_PIXELS;
		return;
	}

	rasterLookup = new int[noOfPixels];

	// if we have a string of pixels just build a flat decode array

	if (pixelSettings.noOfYPixels == 1)
	{
		for (int i = 0; i < pixelSettings.noOfXPixels; i++)
		{
			rasterLookup[i] = i;
		}
	}
	else
	{
		int dest = (pixelSettings.noOfYPixels * pixelSettings.noOfXPixels) - 1;

		for (int y = 0; y < pixelSettings.noOfYPixels; y++)
		{
			for (int x = 0; x < pixelSettings.noOfXPixels; x++)
			{
				int rowStart = y * pixelSettings.noOfXPixels;
				int pos;

				if ((y & 1))
				{
					// even row - ascending order
					pos = rowStart + x;
					rasterLookup[dest] = pos;
				}
				else
				{
					// odd row - descending order
					pos = rowStart + (pixelSettings.noOfXPixels - x - 1);
				}
				rasterLookup[dest] = pos;
				dest--;
			}
		}
	}

	startPixelStrip();
}

void startPixel()
{
	if(bootMode == CONFIG_BOOT_MODE)
	{
		// don't start the pixel display
		pixelProcess.status = PIXELS_STATUS_ONLY;
		return;		
	}

	// Otherwise we create the led storage for the pixels

	int noOfPixels = pixelSettings.noOfXPixels * pixelSettings.noOfYPixels;

	if (noOfPixels == 0)
	{
		pixelProcess.status = PIXEL_NO_PIXELS;
		return;
	}

	leds = new Leds(pixelSettings.noOfXPixels, pixelSettings.noOfYPixels, show, setPixel);

	frame = new Frame(leds, BLACK_COLOUR);
	frame->fadeUp(1000);

	millisOfLastPixelUpdate = millis();
	pixelProcess.status = PIXEL_OK;

	frame->fadeSpritesToWalkingColours("RGBYMC", 10);
	frame->fadeToBrightness(pixelSettings.brightness, 10);
}

void showDeviceStatus();	   // declared in control.h
boolean getInputSwitchValue(); // declared in inputswitch.h

void updateFrame()
{
	unsigned long currentMillis = millis();
	unsigned long millisSinceLastUpdate = ulongDiff(currentMillis, millisOfLastPixelUpdate);

	if (millisSinceLastUpdate >= MILLIS_BETWEEN_UPDATES)
	{
		frame->update();
		frame->render();
		millisOfLastPixelUpdate = currentMillis;
	}
}

void updatePixel()
{
	switch (pixelProcess.status)
	{
	case PIXEL_NO_PIXELS:
		return;
	case PIXEL_OK:
		updateFrame();		
		break;
	case PIXEL_OFF:
		return;
	case PIXELS_STATUS_ONLY:
		return;
	default:
		break;
	}
}

void stopPixel()
{
	pixelProcess.status = PIXEL_OFF;
}

bool pixelStatusOK()
{
	return pixelProcess.status == PIXEL_OK;
}

void pixelStatusMessage(char *buffer, int bufferLength)
{
	switch (pixelProcess.status)
	{
	case PIXEL_NO_PIXELS:
		snprintf(buffer, bufferLength, "No pixels connected");
		break;
	case PIXEL_OK:
		snprintf(buffer, bufferLength, "PIXEL OK");
		break;
	case PIXEL_OFF:
		snprintf(buffer, bufferLength, "PIXEL OFF");
		break;
	case PIXELS_STATUS_ONLY:
		snprintf(buffer, bufferLength, "PIXELS status only");
		break;
	default:
		snprintf(buffer, bufferLength, "Pixel status invalid");
		break;
	}
}

struct process pixelProcess = {
	"pixels",
	initPixel,
	startPixel,
	updatePixel,
	stopPixel,
	pixelStatusOK,
	pixelStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&pixelSettings, sizeof(PixelSettings), &pixelSettingItems,
	&pixelCommands,
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL};
