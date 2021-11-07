#include "Sprite.h"

#include "Frame.h"
#include <math.h>

Frame::Frame(Leds *inLeds, Colour inBackground)
{
	leds = inLeds;

	width = inLeds->ledWidth;
	height = inLeds->ledHeight;
	noOfPixels = width * height;
	overlayActive=false;

	background = inBackground;
	noOfBrightnessSteps = 0;
	brightness = 0;

	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i] = new Sprite(this);
	}
}

Sprite *Frame::getSprite(int spriteNo)
{

	if (spriteNo >= MAX_NO_OF_SPRITES || spriteNo < 0)
	{
		return NULL;
	}

	return sprites[spriteNo];
}

void Frame::overlayColour(Colour col, int timeMins)
{
	overlay = col;
	overlayActive = true;
	overlayEndTicks = millis() + ((60 * timeMins)*1000);
}

void Frame::render()
{
	leds->clear(background);

	if(overlayActive){
		for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
		{
			sprites[i]->renderColour(overlay);
		}
	}
	else {
		for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
		{
			sprites[i]->render();
		}
	}
	leds->display(brightness);
}

void Frame::dump()
{

	Serial.println("\nFrame");
	Serial.printf("Width:%d Height:%d brightness:%f brightness change:%f brightnessSteps:%d\n",
	width, height, brightness, brightnessStep, noOfBrightnessSteps);

	leds->dump();

	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->dump();
	}
}

void Frame::update()
{
	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->update();
	}

	if (noOfBrightnessSteps != 0)
	{
		brightness += brightnessStep;
		noOfBrightnessSteps--;
		if (noOfBrightnessSteps == 0)
		{
			brightness = targetBrightness;
		}
	}

	if(overlayActive){
		if(millis()>overlayEndTicks){
			overlayActive = false;
		}
	}
}

void Frame::fadeUp(int noOfSteps)
{
	fadeToBrightness(1, noOfSteps);
}

void Frame::fadeDown(int noOfSteps)
{
	fadeToBrightness(0, noOfSteps);
}

void Frame::fadeToColour(Colour target, int noOfSteps)
{
	if (noOfSteps <= 0)
	{
		noOfSteps = 10;
	}

	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->fadeToColour(target, noOfSteps);
	}
}

void Frame::setColour(Colour target)
{
	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->setColour(target);
	}
}


void Frame::fadeToBrightness(float inTargetBrightness, int noOfSteps)
{
	if (noOfSteps <= 0)
	{
		noOfSteps = 10;
	}

	if (inTargetBrightness < 0)
	{
		inTargetBrightness = 0;
	}

	if (inTargetBrightness > 1)
	{
		inTargetBrightness = 1;
	}

	brightnessStep = (inTargetBrightness - brightness) / noOfSteps;

	targetBrightness = inTargetBrightness;

	noOfBrightnessSteps = noOfSteps;
}

void Frame::fadeSpritesToColourCharMask(char *colourMask, int steps)
{
	if (steps <= 0)
	{
		steps = 10;
	}

	disableAllSprites();

	char *pos = colourMask;
	int row = 0, col = 0;

	int pixelLimit;

	if (MAX_NO_OF_SPRITES > noOfPixels)
	{
		pixelLimit = noOfPixels;
	}
	else
	{
		pixelLimit = MAX_NO_OF_SPRITES;
	}

	for (int i = 0; i < pixelLimit; i++)
	{
		Sprite *sprite = sprites[i];

		float newX = row + 0.5;
		float newY = col + 0.5;
		// move to the specified position and then stop
		sprite->moveToPosition(newX, newY, steps, Sprite::SPRITE_STOPPED);
		sprite->enabled = true;
		sprite->fadeToBrightness(1, steps);
		sprite->opacity = 1.0;

		setTargetColour(*pos, sprite, steps);

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
	for (int i = 0; i < MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->enabled = false;
	}
}

int Frame::getNumberOfActiveSprites()
{
	int noOfPixels = width * height;

	if (noOfPixels < 12)
		return 3;

	int activeSprites = (int)round(noOfPixels / 4) + 1;

	if (activeSprites > MAX_NO_OF_SPRITES)
	{
		activeSprites = MAX_NO_OF_SPRITES;
	}

	return activeSprites;
}

void Frame::setTargetColour(char ch, Sprite *s, int steps)
{
	switch (ch)
	{
	case '*':
		// A colour of * means "set a random colour for this sprite"
		{
			struct colourNameLookup *newColour = findRandomColour();
			s->fadeToColour(newColour->col, steps);
		}
		break;
	case '+':
		// A colour of + means "don't set the colour of this sprite"
		break;
	default:
		// Any other character, look up the colour and use it
		colourCharLookup *colour = findColourByChar(ch);
		if (colour != NULL)
		{
			s->fadeToColour(colour->col, steps);
		}
		break;
	}
}



void Frame::fadeSpritesToWalkingColours(char *colours, int steps)
{
	if (steps < 0)
		steps = 10;

	char *colourChar = colours;

	int noOfSprites = getNumberOfActiveSprites();

	// fade out and disable all the sprites we aren't using
	for(int i=noOfSprites; i< MAX_NO_OF_SPRITES; i++)
	{
		sprites[i]->fadeToBrightness(0,steps);
	}

	float pixelSpaceBetweenSprites = noOfPixels / noOfSprites;

	float dist = 0;
	int x = 0;
	int y = 0;
	float minSpeed = 0.005;
	float maxSpeed = 0.015;
	float speedStep = (maxSpeed - minSpeed) / (noOfSprites * 2);
	float speed = minSpeed;

	for (int i = 0; i < noOfSprites; i++)
	{
		Sprite *s = sprites[i];
		s->moveToPosition(x + 0.5, y + 0.5, steps, Sprite::SPRITE_WRAP);

		if(random(0,2)==1)
			s->xSpeed = speed;
		else
			s->xSpeed = -speed;
		speed = speed + speedStep;
		if(random(0,2)==1)
			s->ySpeed = speed;
		else
			s->ySpeed = -speed;
		speed = speed + speedStep;
		s->enabled = true;
		
		s->fadeToBrightness(1,steps);
		s->opacity = 1;

		setTargetColour(*colourChar, s, steps);

		colourChar++;

		if (!*colourChar)
		{
			colourChar = colours;
		}

		dist = dist + pixelSpaceBetweenSprites;

		while (dist >= width)
		{
			dist = dist - width;
			y = y + 1;
		}
		x = int(dist);
	}
}

void Frame::fadeSpritesToTwinkle(int steps)
{
	int MAX_NO_OF_SPRITESToTwinkle = getNumberOfActiveSprites();
	for (int i = 0; i < MAX_NO_OF_SPRITESToTwinkle; i++)
	{
		struct colourNameLookup *newColour = findRandomColour();
		sprites[i]->fadeToColour(newColour->col, steps);
	}
}
