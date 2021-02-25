#pragma once

// Forward declaration because Sprite uses Frame
class Frame;

#include "Led.h"
#include "Leds.h"
#include "Sprite.h"

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
	Sprite * sprites;

	Frame(Leds* inLeds, Colour inBackground); 

	Sprite * addSprite(Colour colour, float brightness, float opacity, float x, float y, Updater * updaters);

	void render();

	void update();

	void dump();

	void on();
	void off();

	void fadeUp(float step);

	void fadeDown(float step);
};

