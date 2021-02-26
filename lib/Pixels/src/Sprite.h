#pragma once
#include <math.h>

// Forward declaration because Frame uses Sprite
class Sprite;
class Updater;

#include "Frame.h"
#include "Colour.h"

#include <HardwareSerial.h>

class Updater {
public:

    Updater() {
        nextUpdater = NULL;
    }

    virtual void doUpdate(Sprite* sprite)
    {
    }

    virtual void dump()
    {
    }

    Updater* nextUpdater;
};

class Sprite
{
public:

	Frame* frame;
    Leds* leds;
	Colour colour;
    float redStep, blueStep, greenStep;
    int colourSteps = 0;

	float brightness;
	float opacity;
	float x;
	float y;
    Updater* updater;
    bool enabled;

    void fadeToColour(Colour target, int noOfSteps){
        redStep = (target.Red - colour.Red) / noOfSteps;
        blueStep = (target.Blue - colour.Blue) / noOfSteps;
        greenStep = (target.Green - colour.Green) / noOfSteps;
        colourSteps = noOfSteps;
    }

	void (*doUpdate)(Sprite* sprite);

    void addUpdater(Updater* newUpdater)
    {
        if (updater == NULL) {
            // first sprite is the new head
            updater = newUpdater;
        }
        else {
            // spin down the updaters to find the one at the bottom
            Updater* addPos = updater;
            while (addPos->nextUpdater != NULL) {
                addPos = addPos->nextUpdater;
            }
            addPos->nextUpdater = newUpdater;
        }
    }

    void update()
    {
        if(!enabled)
            return;

        if(colourSteps != 0)
        {
            colour.Red = colour.Red + redStep;
            colour.Blue = colour.Blue + blueStep;
            colour.Green = colour.Green + greenStep;
            colourSteps--;
        }

        Updater* addPos = updater;

        while (addPos != NULL) {
            addPos->doUpdate(this);
            addPos = addPos->nextUpdater;
        }
    }

    void render();

    void dump() {
        Serial.printf("r:%f g:%f b:%f bright:%f opacity:%f x:%f y:%f enabled:%d\n",
            colour.Red, colour.Green, colour.Blue,
            brightness, opacity,enabled,
            x, y);

        Updater* dumpPos = updater;
        while (dumpPos != NULL) {
            Serial.print("    ");
            dumpPos->dump();
            dumpPos = dumpPos->nextUpdater;
        }
    }

    Sprite(Frame * inFrame)
    {
        frame= inFrame;
        enabled=true;
    }

    void enable(){
        enabled=true;
    }

    void disable()
    {
        enabled=false;
    }
    
    void setup(Colour inColour, float inBrightness, float inOpacity,
        float inX, float inY, bool inEnabled, Updater* updaters)
    {
        colour = inColour;
        brightness = inBrightness;
        opacity = inOpacity;
        x = inX;
        y = inY;
        enabled = inEnabled;
        updater = updaters;
    }
};

class BounceMove : public Updater
{
public:
    float speedX;
    float speedY;
    float limitX;
    float limitY;

    BounceMove(float speedX, float speedY, float limitX, float limitY)
    {
        this->speedX = speedX;
        this->speedY = speedY;
        this->limitX = limitX;
        this->limitY = limitY;
    }

    void doUpdate(Sprite* sprite)
    {
        sprite->x = sprite->x + speedX;
        sprite->y = sprite->y + speedY;

        if (sprite->x < 0)
        {
            sprite->x = 0;
            speedX = fabs(speedX);
        }

        if (sprite->y < 0)
        {
            sprite->y = 0;
            speedY = fabs(speedY);
        }

        if (sprite->x > limitX)
        {
            sprite->x = limitX;
            speedX = -fabs(speedX);
        }

        if (sprite->y > limitY)
        {
            sprite->y = limitY;
            speedY = -fabs(speedY);
        }
    }

    void dump()
    {
        Serial.printf("BounceMove sx:%f sy:%f lx:%f ly:%f\n",
            speedX, speedY, limitX, limitY);
    }
};


class WrapMove : public Updater
{
public:

    float speedX;
    float speedY;
    float limitX;
    float limitY;

    WrapMove(float inSpeedX, float inSpeedY, float inLimitX, float inLimitY)
    {
        this->speedX = inSpeedX;
        this->speedY = inSpeedY;
        this->limitX = inLimitX;
        this->limitY = inLimitY;
    }

    void doUpdate(Sprite* sprite)
    {
        sprite->x = sprite->x + speedX;
        sprite->y = sprite->y + speedY;

        if (sprite->x < 0)
        {
            sprite->x = limitX-1;
        }

        if (sprite->y < 0)
        {
            sprite->y = limitY-1;
        }

        if (sprite->x > limitX)
        {
            sprite->x = 0;
        }

        if (sprite->y > limitY)
        {
            sprite->y = 0;
        }
    }

    void dump()
    {
        Serial.printf("WrapMove sx:%f sy:%f lx:%f ly:%f\n",
            speedX, speedY, limitX, limitY);
    }
};



