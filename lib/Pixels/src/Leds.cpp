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

void Leds::renderLight(float sourceX, float sourceY, Colour colour, float brightness, float opacity)
{
	if (ledHeight == 1)
	{
		// linear string of leds - only work in the X axis and fix Y at 0
		// do the leds each side of the draw position

		// this is the led number of the led closest to the light

		int intX = trunc(sourceX);

		for (int xOffset = -1; xOffset <= 1; xOffset++)
		{
			int ledX = intX + xOffset;

			float dist = sourceX - (ledX + 0.5);

			// ensure the distance is always positive
			if(dist<0)
				dist=dist*-1;

			if (dist < 1)
			{
				// we are near enough to light up
				float factor = 1.0 - dist;
				float rs = colour.Red * brightness * factor;
				float gs = colour.Green * brightness * factor;
				float bs = colour.Blue * brightness * factor;

				// wrap the X around for strings
				ledX = ledX % ledWidth;
				if (ledX < 0)
					ledX = ledX + ledWidth;

				leds[ledX][0].AddColourValues(rs, gs, bs, opacity);
			}
		}
	}
	else
	{
		// do the entire 9 leds around the position
		for (int xOffset = -1; xOffset <= 1; xOffset++)
		{
			for (int yOffset = -1; yOffset <= 1; yOffset++)
			{
				int intX = sourceX + xOffset;
				int intY = sourceY + yOffset;
				
				if ((intX >= ledWidth) || (intY >= ledHeight) || (intX < 0) || (intY < 0))
				{
					//				Serial.printf("    ignored\n");
					continue;
				}

				float ledX = intX + 0.5;
				float ledY = intY + 0.5;
				float fx = sourceX - ledX;
				float fy = sourceY - ledY;
				float dist = sqrt((fx * fx) + (fy * fy));

				if (dist < 1)
				{
					// we are near enough to light up
					float factor = 1.0 - dist;
					float rs = colour.Red * brightness * factor;
					float gs = colour.Green * brightness * factor;
					float bs = colour.Blue * brightness * factor;
					leds[intX][intY].AddColourValues(rs, gs, bs, opacity);
				}
			}
		}
	}
}

void Leds::renderDot(float sx, float sy, Colour colour, float brightness, float opacity)
{
	// find the X and Y coordinates of the square containing the pixel

	int intX = trunc(sx);
	int intY = trunc(sy);

	// set the X and Y coordinates of the LED in that square
	// Notionally the leds are in the centre and the squares are 1 unit in size
	float ledX = intX + 0.5;
	float ledY = intY + 0.5;

	// now find the distance from the led for this dot
	float fx = sx - ledX;
	float fy = sy - ledY;
	float dist = sqrt((fx * fx) + (fy * fy));

	//	Serial.printf("sx:%f sy:%f fx:%f fy:%f intX:%d intY:%d ledX:%f ledY:%f dist:%f\n", sx, sy, fx, fy, intX, intY, ledX, ledY, dist);

	// The further the led is away from the dot, the dimmer it should be
	// when the dot is 1 or more units away from the led the led should
	// not light up.

	if (dist < 1)
	{
		// we are near enough to light up
		float factor = 1.0 - dist;
		float rs = colour.Red * brightness * factor;
		float gs = colour.Green * brightness * factor;
		float bs = colour.Blue * brightness * factor;
		leds[intX][intY].AddColourValues(rs, gs, bs, opacity);
		//		Serial.printf("    factor:%f rs:%f gs:%f bs:%f\n", factor, rs, gs, bs );
	}

	// now we need to find the next nearest led and light that
	// we need to find the direction to this led

	float angleInDegrees = atan2(fx, fy) * 180 / 3.141;

	//                       0
	//                   -90    90
	//                      180
	// Use the angle to decide which square is going to be lit up

	// normalise the angle to make it positive:

	if (angleInDegrees < 0)
		angleInDegrees = angleInDegrees + 360;

	//                       0
	//                   270    90
	//                      180
	// Use the angle to decide which square is going to be lit up

	//	Serial.printf("   Angle:%f \n", angleInDegrees);

	// rotate it round 22.5 degrees
	angleInDegrees = angleInDegrees + 22.5;

	// cap the value in case it has wrapped round
	if (angleInDegrees >= 360)
		angleInDegrees = angleInDegrees - 360;

	// now map this onto the destination sector and
	// get the X and Y locations of the destination

	int sector = angleInDegrees / 45;

	//	Serial.printf("sector:%d \n", sector);

	switch (sector)
	{
	case 0:
		// straight up
		intY++;
		break;
	case 1:
		// up right
		intX++;
		intY++;
		break;
	case 2:
		// right
		intX++;
		break;
	case 3:
		// down right
		intX++;
		intY--;
		break;
	case 4:
		// down
		intY--;
		break;
	case 5:
		// down left
		intY--;
		intX++;
		break;
	case 6:
		// left
		intX--;
		break;
	case 7:
		// left up
		intX--;
		intY++;
		break;
	}

	// make sure that the destination led is visible
	if ((intX < ledWidth) && (intY < ledHeight) && (intX >= 0) && (intY >= 0))
	{
		ledX = intX + 0.5;
		ledY = intY + 0.5;
		// now find the distance from the led for this dot
		float fx = sx - ledX;
		float fy = sy - ledY;
		float dist = sqrt((fx * fx) + (fy * fy));

		//		Serial.printf("       intX:%d intY:%d ledX:%f ledY:%f dist:%f\n",intX, intY, ledX, ledY, dist);

		// set the brightness of the new destination
		if (dist < 1)
		{
			// The further the led is away from the dot, the dimmer it should be
			// when the dot is 1 or more units away from the led the led should
			// not light up.
			// we are near enough to light up
			float factor = 1.0 - dist;
			float rs = colour.Red * brightness * factor;
			float gs = colour.Green * brightness * factor;
			float bs = colour.Blue * brightness * factor;
			leds[intX][intY].AddColourValues(rs, gs, bs, opacity);
			//			Serial.printf("            factor:%f rs:%f gs:%f bs:%f\n", factor, rs, gs, bs );
		}
	}
}
