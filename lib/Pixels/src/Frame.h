#pragma once

#include "Led.h"
#include "Leds.h"

#define MAX_NO_OF_SPRITES 25

class Sprite;

class Frame
{
public:

	enum FrameState {ON, OFF, FADE_UP, FADE_DOWN};
	FrameState state;
	int width;
	int height;
	Colour background;
	Leds * leds;
	int noOfPixels;
	float brightness;
	float brightnessStep;
	int noOfBrightnessSteps;
	Sprite ** sprites ;
	int noOfSprites;
	Frame(Leds* inLeds, Colour inBackground); 

	Sprite * getSprite(int spriteNo);

	void render();

	void update();

	void dump();

	void on();
	void off();

	void disableAllSprites();
	void fadeUp(float step);

	void fadeDown(float step);

	bool fadeToBrightness(float brightNess, int steps);

	void fadeToColour(Colour target, int steps);
	void fadeSpritesToColourCharMask(char * colourMask, int steps);
	void fadeSpritesToWalkingColours(char * colours,int steps);
	void fadeSpritesToTwinkle(int steps);
	int getNumberOfActiveSprites();

};

