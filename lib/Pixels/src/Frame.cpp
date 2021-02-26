class Sprite;
class Updater;

#include "Frame.h"

Frame::Frame(Leds * inLeds, Colour inBackground, int inNoOfSprites) {
	leds = inLeds;
	width = inLeds->ledWidth;
	height = inLeds->ledHeight;
	background = inBackground;
	noOfSprites = inNoOfSprites;
	sprites = new Sprite * [noOfSprites];
	for(int i=0;i<noOfSprites;i++)
	{
		sprites[i] = new Sprite(this);
	}
}

bool Frame::setupSprite(int spriteNo, Colour colour, float brightness, float opacity, float x, float y, Updater * updaters)
{
	if(spriteNo >= noOfSprites)
	{
		return false;
	}

//	sprites[spriteNo]->setup(colour, brightness, opacity, x, y, updaters);

	return true;
}

void Frame::render() {

	if (state == OFF) return;

	leds->clear(background);

	for(int i=0;i<noOfSprites;i++)
	{
		sprites[i]->render();
	}

	leds->display(brightness);
}

void Frame::dump() {

	Serial.println("Frame");

	leds->dump();

	for(int i=0;i<noOfSprites;i++)
	{
		sprites[i]->dump();
	}
}

void Frame::update() {

	if (state == OFF) return;

	for(int i=0;i<noOfSprites;i++)
	{
		sprites[i]->update();
	}

	switch (state)
	{
	case FADE_UP:
		brightness += brightnessStep;
		if (brightness >= 1) {
			brightness = 1;
			state = ON;
		}
		break;
	case FADE_DOWN:
		brightness -= brightnessStep;
		if (brightness <= 0) {
			brightness = 0;
			state = OFF;
		}
		break;
	}
}

void Frame::on() {
	brightness = 1;
	state = ON;
}

void Frame::off() {
	brightness = 0;
	state = OFF;
}

void Frame::fadeUp(float step)
{
	brightnessStep = step;
	state = FADE_UP;
}

void Frame::fadeDown(float step)
{
	brightnessStep = step;
	state = FADE_DOWN;
}

