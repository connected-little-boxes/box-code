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

	sprites[spriteNo]->setup(colour, brightness, opacity, x, y, true, updaters);

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

void Frame::fadeToColour(Colour target, int steps){
	for(int i=0;i<noOfSprites;i++)
	{
		sprites[i]->fadeToColour(target, steps);
	}
}

void Frame::fadeSpritesToColourCharMask(char * colourMask, int steps){

	char * pos = colourMask;
	int row=0, col=0;

	for(int i=0;i<noOfSprites;i++)
	{
		Sprite * sprite = sprites[i];

		sprite->x=row+0.5;
		sprite->y=col+0.5;
		sprite->brightness=1.0;
		sprite->opacity=1.0;

		colourCharLookup * colour = findColourByChar(*pos);
		
		if(colour != NULL){
			sprite->fadeToColour(colour->col, steps);
		}

		pos++;

		if(*pos==NULL)
		{
			pos = colourMask;
		}

		row++;
		if(row==width)
		{
			row=0;
			col++;
		}
	}
}


