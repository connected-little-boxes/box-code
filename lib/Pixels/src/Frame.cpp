class Sprite;
class Updater;

#include "Frame.h"

Frame::Frame(Leds * inLeds, Colour inBackground) {
	leds = inLeds;
	width = inLeds->ledWidth;
	height = inLeds->ledHeight;
	background = inBackground;
	state = OFF;
	sprites = NULL;
}

Sprite * Frame::addSprite(Colour colour, float brightness, float opacity, float x, float y, Updater * updaters)
{
	Sprite * newSprite = new Sprite(this, colour, brightness, opacity, x, y, updaters);

	if (sprites == NULL) {
		// first sprite is the new head
		sprites = newSprite;
		return newSprite;
	}
	else {
		// spin down the sprites to find the one at the bottom
		Sprite* addPos = sprites;
		while (addPos->nextSprite != NULL) {
			addPos = addPos->nextSprite;
		}
		addPos->nextSprite = newSprite;
	}
	return newSprite;
}

void Frame::render() {

	if (state == OFF) return;

	leds->clear(background);

	Sprite* currentSprite = sprites;

	while (currentSprite != nullptr) {
		currentSprite->render();
		currentSprite = currentSprite->nextSprite;
	}

	leds->display(brightness);
}

void Frame::dump() {

	Serial.println("Frame");

	leds->dump();

	Sprite* currentSprite = sprites;

	while (currentSprite != nullptr) {
		currentSprite->dump();
		currentSprite = currentSprite->nextSprite;
	}
}

void Frame::update() {

	if (state == OFF) return;

	Sprite* currentSprite = sprites;

	while (currentSprite != nullptr) {
		currentSprite->update();
		currentSprite = currentSprite->nextSprite;
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

