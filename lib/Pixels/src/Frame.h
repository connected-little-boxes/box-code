#pragma once

#include "Led.h"
#include "Leds.h"

#define MAX_NO_OF_SPRITES 20

class Sprite;

class Frame
{
public:

	int width;
	int height;
	int noOfPixels;
	Colour background;
	Colour overlay;
	unsigned long overlayEndTicks;
	bool overlayActive;
	Leds * leds;
	float brightness;
	float brightnessStep;
	int noOfBrightnessSteps;
	float targetBrightness;

	Sprite * sprites [MAX_NO_OF_SPRITES];
	Frame(Leds* inLeds, Colour inBackground); 

	Sprite * getSprite(int spriteNo);

	void overlayColour(Colour col, int timeMins);

	void render();

	void update();

	void dump();

	void disableAllSprites();

	void fadeUp(int noOfSteps);

	void fadeDown(int noOfSteps);

	void fadeToBrightness(float brightness, int steps);
	void setTargetColour(char ch, Sprite * s, int steps);
	void fadeToColour(Colour target, int steps);
	void setColour(Colour target);
	void fadeSpritesToColourCharMask(char * colourMask, int steps);
	void fadeSpritesToWalkingColours(char * colours,int steps);
	void fadeSpritesToTwinkle(int steps);
	int getNumberOfActiveSprites();

};

