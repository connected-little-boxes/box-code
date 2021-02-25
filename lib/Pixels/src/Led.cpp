#include "Led.h"
#include <math.h>

Led::Led()
{
	Reset();
}

void Led::Reset()
{
	colour.Red = 0;
	colour.Green = 0;
	colour.Blue = 0;
}

void Led::AddColour(Colour colour, float fraction) {
	float newVal;

	newVal = colour.Red + (colour.Red * fraction);
	if (newVal > 1) newVal = 1;
	colour.Red = newVal;

	newVal = colour.Green + (colour.Green * fraction);
	if (newVal > 1) newVal = 1;
	colour.Green = newVal;

	newVal = colour.Blue + (colour.Blue *fraction);
	if (newVal > 1) newVal = 1;
	colour.Blue = newVal;
}

void Led::AddColourValues(float r, float g, float b, float opacity) {
	float newRed, newGreen, newBlue;

	if (opacity == 1) {
		newRed = colour.Red + r;
		newGreen = colour.Green + g;
		newBlue = colour.Blue + b;
	}
	else {
		float trans = 1 - opacity;
		newRed = (colour.Red*trans) + r;
		newGreen = (colour.Green * trans) + g;
		newBlue = (colour.Blue * trans) + b;
	}

	if (newRed > 1) newRed = 1;
	colour.Red = newRed;

	if (newGreen > 1) newGreen = 1;
	colour.Green = newGreen;

	if (newBlue > 1) newBlue = 1;
	colour.Blue = newBlue;
}

void Led::ReplceColour(Colour colour, float fraction) {
	float newVal;
	newVal = colour.Red*fraction;
	if (newVal > 1) newVal = 1;
	colour.Red = newVal;

	newVal = colour.Green*fraction;
	if (newVal > 1) newVal = 1;
	colour.Green = newVal;

	newVal = colour.Blue*fraction;
	if (newVal > 1) newVal = 1;
	colour.Blue = newVal;
}

