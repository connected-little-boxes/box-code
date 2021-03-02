#include "Leds.h"
bool close_to(float a, float b);

Leds::Leds(int inWidth, int inHeight, 
			void (*inShow)(),
			void (*inSetPixel)(int no, float r, float g, float b))
{
	ledWidth = inWidth;
	ledHeight = inHeight;

	normWidth = 1.0; // width of display is always 1
	normHeight = (float)ledHeight / (float)ledWidth;

	show = inShow;
	setPixel = inSetPixel;

	leds = new Led *[ledWidth];

	for (int i = 0; i < ledWidth; i++)
	{
		leds[i] = new Led[ledHeight];
	}
}

void Leds::display(float brightness)
{
	int ledNo = 0;
	for (int y = 0; y < ledHeight; y++)
	{
		for (int x = 0; x < ledWidth; x++)
		{
			float r = (leds[x][y].colour.Red * brightness);
			float g = (leds[x][y].colour.Green * brightness);
			float b = (leds[x][y].colour.Blue * brightness);
			setPixel(ledNo, r, g, b);
			ledNo++;
		}
	}
	show();
}

void Leds::clear(Colour colour)
{
	for (int y = 0; y < ledHeight; y++)
	{
		for (int x = 0; x < ledWidth; x++)
		{
			leds[x][y].colour = colour;
		}
	}
}

void Leds::dump()
{
	Serial.printf("Leds width:%d height:%d\n  ", ledWidth, ledHeight);
	for (int y = 0; y < ledHeight; y++)
	{
		for (int x = 0; x < ledWidth; x++)
		{
			Serial.printf("     r:%f g:%f b:%f\n",
						  leds[x][y].colour.Red,
						  leds[x][y].colour.Green,
						  leds[x][y].colour.Blue);
		}
	}
	Serial.println();
}

void Leds::renderLight(float sourceX, float sourceY, Colour colour, float brightness, float opacity)
{

	int intX = int(sourceX);
	int intY = int(sourceY);

	for (int xOffset = -1; xOffset <= 1; xOffset++)
	{
		int px = intX + xOffset;
		int writeX;

		if (px >= ledWidth)
		{
			writeX = px - ledWidth;
		}
		else
		{
			if (px < 0)
			{
				writeX = px + ledWidth;
			}
			else 
			{
				writeX=px;
			}
		}

		for (int yOffset = -1; yOffset <= 1; yOffset++)
		{
			int py = intY + yOffset;
			int writeY;

			if (py >= ledHeight)
			{
				writeY = py - ledHeight;
			}
			else
			{
				if (py < 0)
				{
					writeY = py + ledHeight;
				}
				else
				{
					writeY = py;
				}
			}

			// each pixel is at the centre of a 1*1 square
			// find the distance from this pixel to the light
			float dx = sourceX - (px + 0.5);
			float dy = sourceY - (py + 0.5);
			float dist = sqrt((dx * dx) + (dy * dy));

			// The closer the pixel is to the light - the brighter it is
			// pixels more than 1 from the light are not lit
			if (dist < 1)
			{
				// create a factor from the distance
				float factor = 1 - dist;

				// apply the factor to the colour value
				float rs = colour.Red * brightness * factor;
				float gs = colour.Green * brightness * factor;
				float bs = colour.Blue * brightness * factor;

				leds[writeX][writeY].AddColourValues(rs, gs, bs, opacity);
			}
		}
	}
	return;
}
