
#include "pixels.h"
#include "errors.h"
#include "mqtt.h"
#include "controller.h"
#include "clock.h"

// Some of the colours have been commented out because they don't render well
// on NeoPixels

struct colourLookup colourNames[] = {
{ "black",BLACK_COLOUR },
{ "red",RED_COLOUR },
{ "green",GREEN_COLOUR },
{ "blue",BLUE_COLOUR },
{ "yellow",YELLOW_COLOUR },
{ "orange",ORANGE_COLOUR },
{ "magenta",MAGENTA_COLOUR },
{ "cyan",CYAN_COLOUR },
{ "white",WHITE_COLOUR },
//{ "aquamarine",AQUAMARINE_COLOUR },
{ "azure",AZURE_COLOUR },
//{ "beige",BEIGE_COLOUR },
{ "blueviolet",BLUE_VIOLET_COLOUR },
{ "brown",BROWN_COLOUR },
//{ "burleywood",BURLYWOOD_COLOUR },
//{ "cadetblue",CADET_BLUE_COLOUR },
{ "chartreuse",CHARTREUSE_COLOUR },
//{ "chocolate",CHOCOLATE_COLOUR },
//{ "coral",CORAL_COLOUR },
//{ "cornflowerblue",CORNFLOWER_BLUE_COLOUR },
//{ "cornsilk",CORNSILK_COLOUR },
//{ "darkblue",DARK_BLUE_COLOUR },
//{ "darkcyan",DARK_CYAN_COLOUR },
{ "darkgoldenrod",DARK_GOLDENROD_COLOUR },
//{ "darkgrey",DARK_GRAY_COLOUR },
{ "darkgreen",DARK_GREEN_COLOUR },
//{ "darkkhaki",DARK_KHAKI_COLOUR },
{ "darkmagenta",DARK_MAGENTA_COLOUR },
//{ "darkolive",DARKOLIVE_GREEN_COLOUR },
{ "darkorange",DARK_ORANGE_COLOUR },
//{ "darkorchid",DARK_ORCHID_COLOUR },
{ "darkred",DARK_RED_COLOUR },
//{ "darksalmon",DARK_SALMON_COLOUR },
//{ "darkseagreen",DARK_SEA_GREEN_COLOUR },
//{ "darkslateblue",DARK_SLATE_BLUE_COLOUR },
//{ "darkslategrey",DARK_SLATE_GRAY_COLOUR },
{ "darkturquoise",DARK_TURQUOISE_COLOUR },
{ "darkviolet",DARK_VIOLET_COLOUR },
{ "deeppink",DEEP_PINK_COLOUR },
{ "deepskyblue",DEEP_SKY_BLUE_COLOUR },
//{ "dimgray",DIM_GRAY_COLOUR },
//{ "dodgerblue",DODGERBLUE_COLOUR },
{ "firebrick",FIREBRICK_COLOUR },
//{ "floralwhite",FLORAL_WHITE_COLOUR },
{ "forestgreen",FOREST_GREEN_COLOUR },
//{ "gainsboro",GAINSBORO_COLOUR },
//{ "ghostwhite",GHOST_WHITE_COLOUR },
{ "gold",GOLD_COLOUR },
//{ "goldenrod",GOLDENROD_COLOUR },
//{ "grey",GRAY_COLOUR },
//{ "honeydew",HONEYDEW_COLOUR },
//{ "hotpink",HOT_PINK_COLOUR },
{ "indianred",INDIAN_RED_COLOUR },
//{ "ivory",IVORY_COLOUR },
//{ "khaki",KHAKI_COLOUR },
//{ "lavender",LAVENDER_COLOUR },
//{ "lavenderblush",LAVENDER_BLUSH_COLOUR },
{ "lawngreen",LAWN_GREEN_COLOUR },
//{ "lemon",LEMON_COLOUR },
//{ "lightblue",LIGHT_BLUE_COLOUR },
//{ "lightcoral",LIGHT_CORAL_COLOUR },
//{ "lightcyan",LIGHT_CYAN_COLOUR },
//{ "lightgoldenrod",LIGHT_GOLDENROD_COLOUR },
//{ "lightgrey",LIGHT_GRAY_COLOUR },
//{ "lightgreen",LIGHT_GREEN_COLOUR },
//{ "lightpink",LIGHT_PINK_COLOUR },
//{ "lightsalmon",LIGHT_SALMON_COLOUR },
{ "lightseagreen",LIGHT_SEA_GREEN_COLOUR },
//{ "lightskyblue",LIGHT_SKY_BLUE_COLOUR },
//{ "lightslateblue",LIGHT_SLATE_BLUE_COLOUR },
//{ "lightslategrey",LIGHT_SLATE_GRAY_COLOUR },
//{ "lightsteelblue",LIGHT_STEEL_BLUE_COLOUR },
//{ "lightyellow",LIGHT_YELLOW_COLOUR },
{ "limegreen",LIME_GREEN_COLOUR },
//{ "linen",LINEN_COLOUR },
{ "maroon",MAROON_COLOUR },
//{ "mediumaquamarine",MEDIUM_AQUAMARINE_COLOUR },
{ "mediumblue",MEDIUM_BLUE_COLOUR },
//{ "mediumorchid",MEDIUM_ORCHID_COLOUR },
//{ "mediumpurple",MEDIUM_PURPLE_COLOUR },
//{ "mediumseagreen",MEDIUM_SEA_GREEN_COLOUR },
//{ "mediumslateblue",MEDIUM_SLATE_BLUE_COLOUR },
{ "mediumspringgreen",MEDIUM_SPRING_GREEN_COLOUR },
//{ "mediumturqoise",MEDIUM_TURQUOISE_COLOUR },
{ "mediumviolet",MEDIUM_VIOLET_COLOUR },
{ "midnightblue",MIDNIGHT_BLUE_COLOUR },
//{ "mintcream",MINT_CREAM_COLOUR },
//{ "mistyrose",MISTY_ROSE_COLOUR },
//{ "moccasin",MOCCASIN_COLOUR },
//{ "navajowhite",NAVAJO_WHITE_COLOUR },
{ "navy",NAVY_COLOUR },
//{ "oldlace",OLD_LACE_COLOUR },
//{ "olive",OLIVE_COLOUR },
//{ "olivedrab",OLIVE_DRAB_COLOUR },
{ "orchid",ORCHID_COLOUR },
//{ "palegoldenrod",PALE_GOLDENROD_COLOUR },
//{ "palegreen",PALE_GREEN_COLOUR },
//{ "paleturquoise",PALE_TURQUOISE_COLOUR },
//{ "paleviolet",PALE_VIOLET_COLOUR },
//{ "papayawhip",PAPAYA_WHIP_COLOUR },
//{ "peachpuff",PEACH_PUFF_COLOUR },
//{ "peru",PERU_COLOUR },
//{ "pink",PINK_COLOUR },
//{ "plum",PLUM_COLOUR },
//{ "powderblue",POWDER_BLUE_COLOUR },
{ "purple",PURPLE_COLOUR },
//{ "rosybrown",ROSY_BROWN_COLOUR },
//{ "royalblue",ROYAL_BLUE_COLOUR },
{ "saddlebrown",SADDLE_BROWN_COLOUR },
{ "salmon",SALMON_COLOUR },
//{ "sandybrown",SANDY_BROWN_COLOUR },
{ "seagreen",SEA_GREEN_COLOUR },
//{ "seashell",SEASHELL_COLOUR },
//{ "sienna",SIENNA_COLOUR },
//{ "silver",SILVER_COLOUR },
//{ "skyblue",SKY_BLUE_COLOUR },
//{ "slateblue",SLATE_BLUE_COLOUR },
//{ "slategrey",SLATE_GRAY_COLOUR },
//{ "snow",SNOW_COLOUR },
{ "springgreen",SPRING_GREEN_COLOUR },
//{ "steelblue",STEEL_BLUE_COLOUR },
//{ "tan",TAN_COLOUR },
{ "teal",TEAL_COLOUR },
//{ "thistle",THISTLE_COLOUR },
{ "tomato",TOMATO_COLOUR },
//{ "turqoise",TURQUOISE_COLOUR },
{ "violet",VIOLET_COLOUR },
//{ "wheat",WHEAT_COLOUR },
//{ "whitesmoke",WHITE_SMOKE_COLOUR }
};

int noOfColours = sizeof(colourNames)/sizeof(struct colourLookup);

struct colourLookup *findColourByName(const char *name)
{
	for (unsigned int i = 0; i < sizeof(colourNames) / sizeof(struct colourLookup); i++)
	{
		if (strcasecmp(name, colourNames[i].name) == 0)
		{
			return &colourNames[i];
		}
	}
	return NULL;
}

struct colourLookup *findRandomColour()
{
	// never picks black
	int pos = random(1, sizeof(colourNames) / sizeof(struct colourLookup));
	return &colourNames[pos];
}

Adafruit_NeoPixel *strip = NULL;

struct PixelSettings pixelSettings;

boolean validateColour(void *dest, const char *newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	if (value < 0)
		return false;

	if (value > 255)
		return false;

	return true;
}

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

void setDefaultNoOfPixels(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 12;
}

struct SettingItem pixelNoOfPixelsSetting = {"Number of pixels (0 for pixels not fitted)",
											 "noofpixels",
											 &pixelSettings.noOfPixels,
											 NUMBER_INPUT_LENGTH,
											 integerValue,
											 setDefaultNoOfPixels,
											 validateInt};

void setDefaultPixelConfig(void *dest)
{
	int *destConfig = (int *)dest;
	*destConfig = 1;
}


void setDefaultNoOfVirtualPixels(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 3;
}

boolean validateVirtualPixelConfig(void *dest, const char *newValueStr)
{
	int config;

	bool validConfig = validateInt(&config, newValueStr);

	if (!validConfig)
		return false;

	if (config < 1 || config >= MAX_NO_OF_VIRTUAL_PIXELS)
		return false;

	int *intDest = (int *)dest;

	*intDest = config;

	return true;
}

struct SettingItem pixelNoOfVirtualPixelsSetting = {"Number of virtual pixels",
											 "noofvirtpixels",
											 &pixelSettings.noOfVirtualPixels,
											 NUMBER_INPUT_LENGTH,
											 integerValue,
											 setDefaultNoOfVirtualPixels,
											 validateVirtualPixelConfig};

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
		&pixelNoOfPixelsSetting,
		&pixelNoOfVirtualPixelsSetting,
		&pixelPixelConfig,
};

struct SettingItemCollection pixelSettingItems = {
	"pixel",
	"Pixel hardware and display properties",
	pixelSettingItemPointers,
	sizeof(pixelSettingItemPointers) / sizeof(struct SettingItem *)};

// All the brightness values are between 0 and 1
// Scale them for the particular display

//float overall_brightness = 1.0;

struct Light_Factor
{
	float factor_value, max, min, update;
	void (*do_update)(Light_Factor *factor);
};

#define CLOSE_TOLERANCE 0.01

inline bool close_to(float a, float b)
{
	float diff = a - b;
	if (diff > 0)
	{
		if (diff > CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
	else
	{
		if (diff < -CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
}

boolean coloursEqual(ColourValue a, ColourValue b)
{
	if (!close_to(a.r, b.r))
		return false;
	if (!close_to(a.g, b.g))
		return false;
	if (!close_to(a.b, b.b))
		return false;
	return true;
}

void do_no_update(Light_Factor *target)
{
}

void do_update_larsen(Light_Factor *target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->max;
		target->update = -target->update;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->min;
		target->update = -target->update;
	}
}

void do_update_loop(Light_Factor *target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->min;
		return;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->max;
	}
}

void dump_light_factor(Light_Factor *factor)
{
	Serial.print("Value: ");
	Serial.print(factor->factor_value);
	Serial.print(" Max: ");
	Serial.print(factor->max);
	Serial.print(" Min: ");
	Serial.print(factor->min);
	Serial.print(" Update: ");
	Serial.print(factor->update);
}

#define RED_FACTOR 0
#define GREEN_FACTOR (RED_FACTOR + 1)
#define BLUE_FACTOR (GREEN_FACTOR + 1)
#define FLICKER_FACTOR (BLUE_FACTOR + 1)
#define POSITION_FACTOR (FLICKER_FACTOR + 1)
#define WIDTH_FACTOR (POSITION_FACTOR + 1)
#define BRIGHTNESS_FACTOR (WIDTH_FACTOR+1)
#define NO_OF_FACTORS (BRIGHTNESS_FACTOR + 1)

char *factor_names[] = {"Red", "Green", "Blue", "Flicker", "Position", "Width", "Brightness"};

struct VirtualPixel
{
	Light_Factor factors[NO_OF_FACTORS];
};

struct VirtualPixel lamps[MAX_NO_OF_VIRTUAL_PIXELS];

void dumpVirtualPixel(VirtualPixel *lamp)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Serial.print("   ");
		Serial.print(factor_names[i]);
		Serial.print(": ");
		dump_light_factor(&lamp->factors[i]);
		Serial.println();
	}
}

void dumpVirtualPixels(struct VirtualPixel *lamps)
{
	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		dumpVirtualPixel(&lamps[i]);
		Serial.println();
	}
}

struct Pixel
{
	byte r, g, b;
};

struct Pixel pixels[MAX_NO_OF_PIXELS];

void clear_pixels()
{
	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		pixels[i].r = 0;
		pixels[i].g = 0;
		pixels[i].b = 0;
	}
}

byte clamp_colour(int c)
{
	if (c > 255)
		return 255;
	if (c < 0)
		return 0;
	return c;
}

void add_color_to_pixel(int pos, int r, int g, int b)
{
	byte newr = clamp_colour((int)pixels[pos].r + r);
	byte newg = clamp_colour((int)pixels[pos].g + g);
	byte newb = clamp_colour((int)pixels[pos].b + b);

	pixels[pos].r = newr;
	pixels[pos].g = newg;
	pixels[pos].b = newb;
}

void renderSingleVirtualPixel(VirtualPixel *lamp)
{
	float brightness = lamp->factors[FLICKER_FACTOR].factor_value * lamp->factors[BRIGHTNESS_FACTOR].factor_value * 255;

	// Map the position value from 360 degrees to a pixel number

	float pixel_pos = (lamp->factors[POSITION_FACTOR].factor_value / 360.0) * pixelSettings.noOfPixels;

	int pos = (int)(pixel_pos);

	float diff = pixel_pos - pos;

	float low_factor = 1 - diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * low_factor);

	add_color_to_pixel(pos, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * diff);

	add_color_to_pixel((pos + 1) % pixelSettings.noOfPixels, r, g, b);
}

void renderVirtualPixel(VirtualPixel *lamp)
{
	if (lamp->factors[WIDTH_FACTOR].factor_value <= 1)
	{
		renderSingleVirtualPixel(lamp);
		return;
	}

	float brightness = lamp->factors[FLICKER_FACTOR].factor_value * lamp->factors[BRIGHTNESS_FACTOR].factor_value * 255;

	// Map the position value from 360 degrees to a pixel number

	float half_width = lamp->factors[WIDTH_FACTOR].factor_value / 2.0;
	float pos = lamp->factors[POSITION_FACTOR].factor_value;
	float left_pos = pos - half_width;
	float right_pos = pos + half_width;

	float left_pixel_pos = (left_pos / 360 * pixelSettings.noOfPixels);
	float right_pixel_pos = (right_pos / 360 * pixelSettings.noOfPixels);

	int left_int_pos = (int)(left_pixel_pos);
	int right_int_pos = (int)(right_pixel_pos);

	for (int i = left_int_pos; i <= right_int_pos; i++)
	{
		byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness);
		byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness);
		byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness);

		add_color_to_pixel(i, r, g, b);
	}

	float left_diff = left_pixel_pos - left_int_pos;

	float left_low_factor = 1 - left_diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * left_low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * left_low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * left_low_factor);

	add_color_to_pixel(left_int_pos - 1, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * left_diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * left_diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * left_diff);

	//	add_color_to_pixel((left_int_pos) % settings.noOfPixels, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));

	float right_diff = right_pixel_pos - right_int_pos;

	float right_low_factor = 1 - right_diff;

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * right_diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * right_diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * right_diff);

	add_color_to_pixel(right_int_pos + 1, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * right_low_factor);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * right_low_factor);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * right_low_factor);

	//	add_color_to_pixel((right_int_pos) % settings.noOfPixels, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));
}

void setPixelFromStruct(int pixel, struct ColourValue colour)
{
	if (strip == NULL)
		return;

	strip->setPixelColor(pixel, colour.r, colour.g, colour.b);
}

void renderVirtualPixels(struct VirtualPixel *lamps)
{
	if (strip == NULL)
		return;

	clear_pixels();

	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		renderVirtualPixel(&lamps[i]);
	}

	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		// Serial.printf("p:%d r:%x g:%x b:%x  ", i, pixels[i].r, pixels[i].g, pixels[i].b);

		strip->setPixelColor(i, pixels[i].r, pixels[i].g, pixels[i].b);
	}

	// Serial.println();
	strip->show();
}

void updateVirtualPixel(VirtualPixel *target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Light_Factor *factor = &target->factors[i];
		factor->do_update(factor);
	}
}

void updateVirtualPixels(struct VirtualPixel *lamps)
{
	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		updateVirtualPixel(&lamps[i]);
	}
	renderVirtualPixels(lamps);
}

void clear_factor(Light_Factor *target)
{
	target->factor_value = 0;
	target->max = 1.0;
	target->min = 0.0;
	target->update = 0.0;
	target->do_update = do_no_update;
}

void clearVirtualPixel(VirtualPixel *target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		clear_factor(&target->factors[i]);
	}
}

void clearVirtualPixels(struct VirtualPixel *lamps)
{
	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		clearVirtualPixel(&lamps[i]);
	}
}

void setupVirtualPixel(VirtualPixel *target, float r, float g, float b, float pos, float width, float flicker, float brightness)
{
	clearVirtualPixel(target);
	target->factors[RED_FACTOR].factor_value = r;
	target->factors[GREEN_FACTOR].factor_value = g;
	target->factors[BLUE_FACTOR].factor_value = b;
	target->factors[POSITION_FACTOR].factor_value = pos;
	target->factors[WIDTH_FACTOR].factor_value = width;
	target->factors[FLICKER_FACTOR].factor_value = flicker;
	target->factors[BRIGHTNESS_FACTOR].factor_value = brightness;
}

void setupVirtualPixelFactor(VirtualPixel *target, byte factor_number, float factor_value, float min, float max, float update, void (*do_update)(Light_Factor *f))
{
	target->factors[factor_number].factor_value = factor_value;
	target->factors[factor_number].max = max;
	target->factors[factor_number].min = min;
	target->factors[factor_number].update = update;
	target->factors[factor_number].do_update = do_update;
}

// If the pixels are illuminating a message these are the numbers of the
// pixels that are actually in the message
// Need to change this is the message changes.

void setupWalkingColour(ColourValue colour)
{
	if (pixelSettings.noOfPixels == 0)
		return;

	float start_speed = 0.125;
	//	float speed_update = 0.125;    // speed for the desktop sensor
	float speed_update = .25; // speed for the top hat

	float degreesPerPixel = 360.0 / pixelSettings.noOfVirtualPixels;

	clearVirtualPixels(lamps);

	for (int i = 0; i < MAX_NO_OF_VIRTUAL_PIXELS; i++)
	{
		int pos = i * degreesPerPixel;
		// Serial.printf("Pixel: %d position:%d", i, pos);

		setupVirtualPixel(&lamps[i], colour.r, colour.g, colour.b, pos, 0, 1.0,1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, pos, 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}

	//dumpVirtualPixels(lamps);
	renderVirtualPixels(lamps);
}

float calculateStepSize(float start, float end, int noOfSteps)
{
	float range = end - start;
	return range / noOfSteps;
}

void do_update_fade(Light_Factor *target)
{
	target->factor_value += target->update;
	if (close_to(target->factor_value, target->max))
	{
		target->factor_value = target->max;
		target->do_update = do_no_update;
	}
}

void startFade(Light_Factor *factor, float targetValue, int noOfsteps)
{
	if (close_to(factor->factor_value, targetValue))
		return;

	factor->update = calculateStepSize(factor->factor_value, targetValue, noOfsteps);
	factor->max = targetValue;
	factor->do_update = do_update_fade;
}

void fadeWalkingColour(ColourValue newColour, int noOfSteps)
{
	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		startFade(&lamps[i].factors[RED_FACTOR], newColour.r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColour.g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColour.b, noOfSteps);
	}
}

void changeWalkingColour(ColourValue colour)
{
	fadeWalkingColour(colour, 20);
}

void fadeWalkingColours(ColourValue *newColours, int noOfColours, int noOfSteps)
{
	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		int colourNo = i % noOfColours;
		startFade(&lamps[i].factors[RED_FACTOR], newColours[colourNo].r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColours[colourNo].g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColours[colourNo].b, noOfSteps);
	}
}

unsigned long millisOfLastPixelUpdate;

void startPixelStrip()
{
	switch (pixelSettings.pixelConfig)
	{
	case 1:
		strip = new Adafruit_NeoPixel(pixelSettings.noOfPixels, pixelSettings.pixelControlPinNo,
									  NEO_GRB + NEO_KHZ800);
		break;
	case 2:
		strip = new Adafruit_NeoPixel(pixelSettings.noOfPixels, pixelSettings.pixelControlPinNo,
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

// status display doesn't use the animated leds
// this means that it can overlay the display

int statusPixelNo = 0;

void initialiseStatusDisplay(byte r, byte g, byte b)
{
	if (pixelSettings.noOfPixels == 0)
		return;

	statusPixelNo = 0;

	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		setPixelFromStruct(i, {(float)r, (float)g, (float)b});
	}

	renderStatusDisplay();
}

boolean setStatusDisplayPixel(int pixelNumber, boolean statusOK)
{
	if (pixelSettings.noOfPixels == 0)
		return false;

	if (pixelNumber >= pixelSettings.noOfPixels)
		return false;

	if (statusOK)
	{
		setPixelFromStruct(pixelNumber, {0, 128, 0});
	}
	else
	{
		setPixelFromStruct(pixelNumber, {128, 0, 0});
	}

	return true;
}

void beginStatusDisplay()
{
	initialiseStatusDisplay(0, 10, 0);
}

void beginWifiStatusDisplay()
{
	initialiseStatusDisplay(10, 0, 10);
}

void renderStatusDisplay()
{
	if (strip == NULL)
		return;

	strip->show();
	delay(200);
}

boolean addStatusItem(boolean status)
{
	if (statusPixelNo >= pixelSettings.noOfPixels)
		return false;

	setStatusDisplayPixel(statusPixelNo, status);

	statusPixelNo++;

	return true;
}

/// commands

boolean validatePixelCommandString(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, PIXEL_COMMAND_NAME_LENGTH));
}

// The first element of a process command block is always a floating point value called "value"
// This will be loaded with a normalised value to be used by the process when it runs

#define FLOAT_VALUE_OFFSET 0
#define RED_PIXEL_COMMAND_OFFSET (FLOAT_VALUE_OFFSET+sizeof(float))
#define BLUE_PIXEL_COMMAND_OFFSET (RED_PIXEL_COMMAND_OFFSET + sizeof(float))
#define GREEN_PIXEL_COMMAND_OFFSET (BLUE_PIXEL_COMMAND_OFFSET + sizeof(float))
#define SPEED_PIXEL_COMMAND_OFFSET (GREEN_PIXEL_COMMAND_OFFSET + sizeof(float))
#define COMMAND_PIXEL_COMMAND_OFFSET (SPEED_PIXEL_COMMAND_OFFSET + sizeof(int))
#define COLOURNAME_PIXEL_COMMAND_OFFSET (COMMAND_PIXEL_COMMAND_OFFSET + PIXEL_COMMAND_NAME_LENGTH)
#define COMMAND_PIXEL_OPTION_OFFSET (COLOURNAME_PIXEL_COMMAND_OFFSET+PIXEL_COMMAND_NAME_LENGTH)

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

boolean setDefaultPixelSpeed(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 20;
	return true;
}


struct CommandItem speedCommandItem = {
	"pixelSpeed",
	"change speed in 50ths of a second",
	SPEED_PIXEL_COMMAND_OFFSET,
	integerCommand,
	validateInt,
	setDefaultPixelSpeed};

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

struct CommandItem colourCommandOptionItem = {
	"option",
	"option for the colour command(timed)",
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

struct CommandItem *setColourItems[] =
	{
		&redCommandItem,
		&blueCommandItem,
		&greenCommandItem,
		&speedCommandItem};

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
		return publishBufferToMQTTTopic(buffer, destination);
	}

	// sets the pixel colour - caller has set the r,g and b values

	float red = (float) getUnalignedFloat(settingBase + RED_PIXEL_COMMAND_OFFSET);
	float blue = (float) getUnalignedFloat(settingBase + BLUE_PIXEL_COMMAND_OFFSET);
	float green = (float) getUnalignedFloat(settingBase + GREEN_PIXEL_COMMAND_OFFSET);
	int steps = (int) getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	fadeWalkingColour({red, green, blue}, steps);

	return WORKED_OK;
}

struct CommandItem *setNamedPixelColourItems[] =
	{
		&colourCommandName,
		&speedCommandItem};

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
		return publishBufferToMQTTTopic(buffer, destination);
	}

	struct colourLookup *col;

	char *colourName = (char *)(settingBase + COLOURNAME_PIXEL_COMMAND_OFFSET);

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	col = findColourByName(colourName);

	if (col != NULL)
	{
		fadeWalkingColour(col->col, steps);
		return WORKED_OK;
	}
	else
	{
		return JSON_MESSAGE_INVALID_COLOUR_NAME;
	}
}

struct CommandItem *setRandomPixelColourItems[] =
	{
		&speedCommandItem,
		&colourCommandOptionItem};

int doSetRandomColour(char *destination, unsigned char *settingBase);

struct Command setPixelsToRandomColour
{
	"setrandomcolour",
		"Sets the pixels to a random colour",
		setRandomPixelColourItems,
		sizeof(setRandomPixelColourItems) / sizeof(struct CommandItem *),
		doSetRandomColour
};

void seedRandomFromClock()
{
	if (clockSensor.status != SENSOR_OK)
	{
		// no clock - just pick a random colour
		return ;
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
		return publishBufferToMQTTTopic(buffer, destination);
	}

	char *option = (char *)(settingBase + COMMAND_PIXEL_OPTION_OFFSET);

	if (strcasecmp(option, "timed") == 0)
	{
		seedRandomFromClock();
	}

	struct colourLookup *randomColour = findRandomColour();

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	fadeWalkingColour(randomColour->col, steps);

	return WORKED_OK;
}

struct CommandItem *setPixelTwinkleItems[] =
	{
		&speedCommandItem,
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
		return publishBufferToMQTTTopic(buffer, destination);
	}

	char *option = (char *)(settingBase + COMMAND_PIXEL_OPTION_OFFSET);

	if (strcasecmp(option, "timed") == 0)
	{
		seedRandomFromClock();
	}

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		struct colourLookup *newColour = findRandomColour();
		startFade(&lamps[i].factors[RED_FACTOR], newColour->col.r, steps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColour->col.g, steps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColour->col.b, steps);
	}

	return WORKED_OK;
}

struct CommandItem * setPixelBrightnessItems [] =
{
	&floatValueItem,
	&speedCommandItem
};

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
		return publishBufferToMQTTTopic(buffer, destination);
	}

	float brightness = getUnalignedFloat(settingBase + FLOAT_VALUE_OFFSET);

	int steps = getUnalignedInt(settingBase + SPEED_PIXEL_COMMAND_OFFSET);

	for (int i = 0; i < pixelSettings.noOfVirtualPixels; i++)
	{
		startFade(&lamps[i].factors[BRIGHTNESS_FACTOR], brightness, steps);
	}

	return WORKED_OK;
}

struct Command *pixelCommandList[] = {
	&setPixelColourCommand,
	&setPixelsToNamedColour,
	&setPixelsToRandomColour,
	&setPixelsToTwinkle,
	&setPixelBrightness};

struct CommandItemCollection pixelCommands =
	{
		"Control the pixels on the device",
		pixelCommandList,
		sizeof(pixelCommandList) / sizeof(struct Command *)};

void initPixel()
{
	pixelProcess.status = PIXEL_OFF;
}

void startPixel()
{
	if (pixelSettings.noOfPixels == 0)
	{
		pixelProcess.status = PIXEL_ERROR_NO_PIXELS;
	}

	startPixelStrip();

	millisOfLastPixelUpdate = millis();
	pixelProcess.status = PIXEL_OK;
}

void showDeviceStatus();	   // declared in control.h
boolean getInputSwitchValue(); // declared in inputswitch.h

void updatePixel()
{
	if (pixelSettings.noOfPixels == 0)
		return;

	if (pixelProcess.status != PIXEL_OK)
	{
		return;
	}

	unsigned long currentMillis = millis();
	unsigned long millisSinceLastUpdate = ulongDiff(currentMillis, millisOfLastPixelUpdate);

	if (millisSinceLastUpdate >= MILLIS_BETWEEN_UPDATES)
	{
		updateVirtualPixels(lamps);
		millisOfLastPixelUpdate = currentMillis;
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
	if (pixelSettings.noOfPixels == 0)
	{
		snprintf(buffer, bufferLength, "No pixels connected");
		return;
	}

	switch (pixelProcess.status)
	{
	case PIXEL_OK:
		snprintf(buffer, bufferLength, "PIXEL OK");
		break;
	case PIXEL_OFF:
		snprintf(buffer, bufferLength, "PIXEL OFF");
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
	NULL,
	(unsigned char *)&pixelSettings, sizeof(PixelSettings), &pixelSettingItems,
	&pixelCommands,
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL};
