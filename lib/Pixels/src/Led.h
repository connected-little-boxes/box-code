#pragma once

#include "Colour.h"

class Led
{
public:

	Led();

	Colour colour;

	void Reset();
	void AddColour(Colour colour, float fraction);
	void AddColourValues(float r, float g, float b, float opacity);
	void ReplceColour(Colour colour, float fraction);


};

