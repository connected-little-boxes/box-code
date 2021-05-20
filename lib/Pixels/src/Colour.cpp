#include "Colour.h"

#include <string.h>
#include <Arduino.h>

struct colourNameLookup colourNames[] = {
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

struct colourCharLookup colourChars[] = {
{ 'K',BLACK_COLOUR },
{ 'R',RED_COLOUR },
{ 'G',GREEN_COLOUR },
{ 'B',BLUE_COLOUR },
{ 'Y',YELLOW_COLOUR },
{ 'O',ORANGE_COLOUR },
{ 'M',MAGENTA_COLOUR },
{ 'C',CYAN_COLOUR },
{ 'W',WHITE_COLOUR },
{ 'A',AZURE_COLOUR },
{ 'V',VIOLET_COLOUR },
{ 'P',PURPLE_COLOUR },
{ 'T',TEAL_COLOUR }
};

int noOfColours = sizeof(colourNames)/sizeof(struct colourNameLookup);

struct colourNameLookup *findColourByName(const char *name)
{
	for (unsigned int i = 0; i < sizeof(colourNames) / sizeof(struct colourNameLookup); i++)
	{
		if (strcasecmp(name, colourNames[i].name) == 0)
		{
			return &colourNames[i];
		}
	}
	return NULL;
}

struct colourCharLookup *findColourByChar(const char ch)
{
	for (unsigned int i = 0; i < sizeof(colourChars) / sizeof(struct colourCharLookup); i++)
	{
		if (colourChars[i].ch == ch)
		{
			return &colourChars[i];
		}
	}
	return NULL;
}

int localRand(int low, int high);

struct colourNameLookup *findRandomColour()
{
	// never picks black - which is at location 0 in the array
	int pos = localRand(1, sizeof(colourNames) / sizeof(struct colourNameLookup));
	return &colourNames[pos];
}

void getColourInbetweenMask(char lowChar, char highChar, float distance, Colour * result)
{
	struct Colour from = findColourByChar(lowChar)->col;
	struct Colour to = findColourByChar(highChar)->col;
	result->Red = from.Red + (to.Red - from.Red * distance);
	result->Green = from.Green + (to.Green - from.Green * distance);
	result->Blue = from.Blue + (to.Blue - from.Blue * distance);
}

