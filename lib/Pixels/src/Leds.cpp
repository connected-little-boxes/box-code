#include "Leds.h"
bool close_to(float a, float b);

//#define DUMP_LED_VALUES

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
			Serial.printf("r:%f g:%f b:%f  ",
						  leds[x][y].colour.Red,
						  leds[x][y].colour.Green,
						  leds[x][y].colour.Blue);
		}
	}
	Serial.println();
}

// use the direction of movement to decide which dot to light up next....

/*
 void Leds::renderDot(float sx, float sy, float oldX, float oldY, Colour colour, float brightness, float opacity)
{
	int ledX= (int)sx;
	int ledY = (int)sy;

	float pX = ledX+0.5;
	float pY = ledY+0.5;

	float dx = ledX-pX;
	float dy = ledY-pY;

	float dist=sqrt((dx*dx)+(dy*dy));
	// factor is 1 when the distance is 0 and 0 when distance is 0.707 (half of root 2)

	float distanceBrightness = 0.707-dist;

	float rs = colour.Red * brightness * distanceBrightness;
	float gs = colour.Green * brightness * distanceBrightness;
	float bs = colour.Blue * brightness * distanceBrightness;

	leds[ledX][ledY].AddColourValues(rs, gs, bs, opacity);

	if(close_to(sx,oldX)&&close_to(sy,oldY)){
		return;
	}

	// now find the 

}
*/

void Leds::renderDot(float sx, float sy, float oldX, float oldY, Colour colour, float brightness, float opacity)
{
	int intX = trunc(sx);
	int intY = trunc(sy);

	float dx = (sx - intX) / 4.0;
	float dy = (sy - intY) / 4.0;

	float nx = 0.25 - dx;
	float ny = 0.25 - dy;

	int nextX = intX + 1;
	int nextY = intY + 1;

	// Will sum the X and Y components to get the
	// final colour level

	float rs = colour.Red * brightness;
	float gs = colour.Green * brightness;
	float bs = colour.Blue * brightness;

#ifdef DUMP_LED_VALUES
	Serial.printf("x:%f y:%f ix:%d iy:%d nx:%d ny:%d r:%f g:%f b:%f\n",
				  sx, sy, intX, intY, nextX, nextY, rs, gs, bs);
	Serial.printf("dx:%f dy:%f nx:%f ny:%f\n", dx, dy, nx, ny);
#endif

	float r, g, b;

	r = (rs * nx) + (rs * ny);
	g = (gs * nx) + (gs * ny);
	b = (bs * nx) + (bs * ny);

	leds[intX][intY].AddColourValues(r, g, b, opacity);
#ifdef DUMP_LED_VALUES
	Serial.printf("    ix,iy r:%f g:%f b:%f\n", r, g, b);
#endif

	if (nextY < ledHeight)
	{
		r = (rs * nx) + (rs * dy);
		g = (gs * nx) + (gs * dy);
		b = (bs * nx) + (bs * dy);

		leds[intX][nextY].AddColourValues(r, g, b, opacity);
#ifdef DUMP_LED_VALUES
		Serial.printf("    ix,ny r:%f g:%f b:%f\n", r, g, b);
#endif
	}

	if ((nextX < ledWidth) && (nextY < ledHeight))
	{
		r = (rs * dx) + (rs * dy);
		g = (gs * dx) + (gs * dy);
		b = (bs * dx) + (bs * dy);

		leds[nextX][nextY].AddColourValues(r, g, b, opacity);
#ifdef DUMP_LED_VALUES
		Serial.printf("    nx,iy r:%f g:%f b:%f\n", r, g, b);
#endif
	}

	if (nextX < ledWidth)
	{
		r = (rs * dx) + (rs * ny);
		g = (gs * dx) + (gs * ny);
		b = (bs * dx) + (bs * ny);

		leds[nextX][intY].AddColourValues(r, g, b, opacity);
#ifdef DUMP_LED_VALUES
		Serial.printf("    nx,ny r:%f g:%f b:%f\n", r, g, b);
#endif
	}
}
