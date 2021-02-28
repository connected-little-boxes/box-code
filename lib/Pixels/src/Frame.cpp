#include "Sprite.h"

#include "Frame.h"

Frame::Frame(Leds *inLeds, Colour inBackground, int inNoOfSprites)
{
	leds = inLeds;
	width = inLeds->ledWidth;
	height = inLeds->ledHeight;
	background = inBackground;
	noOfSprites = inNoOfSprites;
	sprites = new Sprite *[noOfSprites];
	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i] = new Sprite(this);
	}
}

Sprite *Frame::getSprite(int spriteNo)
{

	if (spriteNo >= noOfSprites || spriteNo < 0)
	{
		return NULL;
	}

	return sprites[spriteNo];
}

void Frame::render()
{

	if (state == OFF)
		return;

	leds->clear(background);

	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i]->render();
	}

	leds->display(brightness);
}

void Frame::dump()
{

	Serial.println("Frame");

	leds->dump();

	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i]->dump();
	}
}

void Frame::update()
{

	if (state == OFF)
		return;

	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i]->update();
	}

	switch (state)
	{
	case FADE_UP:
		brightness += brightnessStep;
		if (brightness >= 1)
		{
			brightness = 1;
			state = ON;
		}
		break;
	case FADE_DOWN:
		brightness -= brightnessStep;
		if (brightness <= 0)
		{
			brightness = 0;
			state = OFF;
		}
		break;
	case ON:
		if (noOfBrightnessSteps != 0)
		{
			brightness += brightnessStep;
			noOfBrightnessSteps--;
		}
		break;
	case OFF:
		break;
	}
}

void Frame::on()
{
	brightness = 1;
	state = ON;
}

void Frame::off()
{
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

void Frame::fadeToColour(Colour target, int noOfSteps)
{
	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i]->fadeToColour(target, noOfSteps);
	}
}

bool Frame::fadeToBrightness(float targetBrightness, int noOfSteps)
{
	if (noOfSteps <= 0 || brightness < 0 || brightness > 1)
	{
		return false;
	}

	brightnessStep = (targetBrightness - brightness) / noOfSteps;

	noOfBrightnessSteps = noOfSteps;
	return true;
}

void Frame::fadeSpritesToColourCharMask(char *colourMask, int steps)
{
	disableAllSprites();

	char *pos = colourMask;
	int row = 0, col = 0;

	int noOfPixels = width * height;
	int pixelLimit;

	if (noOfSprites > noOfPixels)
	{
		pixelLimit = noOfPixels;
	}
	else
	{
		pixelLimit = noOfSprites;
	}

	for (int i = 0; i < pixelLimit; i++)
	{
		Sprite *sprite = sprites[i];

		float newX = row + 0.5;
		float newY = col + 0.5;
		sprite->moveToPosition(newX, newY, steps);
		sprite->enabled = true;
		sprite->brightness = 1.0;
		sprite->opacity = 1.0;

		if (*pos == '+')
		{
			colourNameLookup *colourName = findRandomColour();
			sprite->fadeToColour(colourName->col, steps);
		}
		else
		{
			colourCharLookup *colour = findColourByChar(*pos);

			if (colour != NULL)
			{
				sprite->fadeToColour(colour->col, steps);
			}
		}

		pos++;

		// loop around the string
		if (*pos == 0)
		{
			pos = colourMask;
		}

		row++;
		if (row == width)
		{
			row = 0;
			col++;
		}
	}
}

void Frame::disableAllSprites()
{
	for (int i = 0; i < noOfSprites; i++)
	{
		sprites[i]->enabled = false;
	}
}

void Frame::fadeSpritesToWalkingColours(char *colours, int steps)
{
	if (steps < 0)
		steps = 10;

	int noOfSpritesToWalk = noOfSprites / 4;

	if (noOfSpritesToWalk > noOfSprites)
	{
		noOfSpritesToWalk = noOfSprites;
	}

	char *colourChar = colours;

	float xSpeed = 0.01;
	float speedStep = 0.01;

	disableAllSprites();

	for (int i = 0; i < noOfSpritesToWalk; i++)
	{
		Sprite *s = sprites[i];

		// first get the target colour
		// A colour of * means "don't set the colour of this pixel"
		if (*colourChar != '*')
		{
			colourCharLookup *colour = findColourByChar(*colourChar);
			if (colour != NULL)
			{
				s->fadeToColour(colour->col, steps);
			}
		}

		s->enabled = true;
		s->brightness = 1;
		s->opacity = 1;
		s->xSpeed = xSpeed + (i * speedStep);
		s->ySpeed = xSpeed + (i * speedStep);
		s->enabled = true;
		s->movingState = Sprite::SPRITE_WRAP;

		colourChar++;

		if (!*colourChar)
		{
			colourChar = colours;
		}
	}
}
