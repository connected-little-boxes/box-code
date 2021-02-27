#pragma once
#include "Led.h"
#include "math.h"
#include <HardwareSerial.h>

class Leds
{
public:

	void(*show)();
	void(*setPixel)(int no, float r, float g, float b);

	Led** leds;
	int ledWidth;
	int ledHeight;

	float normWidth;
	float normHeight;

	Leds(int inWidth, int inHeight, 
		void(*inShow)(), 
		void(*inSetPixel)(int no, float r, float g, float b));

	void dump();
	void display(float brightness);
	void clear(Colour colour);
	void renderDot(float sx, float sy, Colour colour, float brightness, float opacity);
	void renderLight(float sourceX, float sourceY, Colour colour, float brightness, float opacity);

};

