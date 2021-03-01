#pragma once

#include <Arduino.h>
#include <math.h>

class Frame;
class Leds;

#include "Colour.h"

class Sprite
{
public:

    enum SpriteMovingState {SPRITE_STOPPED, SPRITE_BOUNCE, SPRITE_WRAP, SPRITE_MOVE};

    SpriteMovingState movingState;

	Frame* frame;
    Leds* leds;
	Colour colour;
    float redStep, blueStep, greenStep;
    int colourSteps = 0;

	float brightness;
	float opacity;

	float x;
	float y;
    float xSpeed;
    float ySpeed;
    int moveSteps=0;

    bool enabled;

    void fadeToColour(Colour target, int noOfSteps){
        redStep = (target.Red - colour.Red) / noOfSteps;
        blueStep = (target.Blue - colour.Blue) / noOfSteps;
        greenStep = (target.Green - colour.Green) / noOfSteps;
        colourSteps = noOfSteps;
    }

    void moveToPosition(float targetX, float targetY, int noOfSteps)
    {
        xSpeed = (targetX-x) / noOfSteps;
        ySpeed = (targetY-y) / noOfSteps;
        moveSteps = noOfSteps;
        movingState = SPRITE_MOVE;
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

        switch(movingState)
        {
            case SPRITE_STOPPED:
                break;
            case SPRITE_BOUNCE:
                bounce();
                break;
            case SPRITE_WRAP:
                wrap();
                break;
            case SPRITE_MOVE:
                move();
                break;
        }
    }

    void render();

    void bounce();

    void wrap();

    void move();

    void dump() {
        Serial.printf("r:%f g:%f b:%f bright:%f opacity:%f enabled:%d x:%f y:%f xspeed:%f yspeed:%f",
            colour.Red, colour.Green, colour.Blue,
            brightness, opacity,enabled,
            x, y, xSpeed, ySpeed);

        switch(movingState)
        {
            case SPRITE_STOPPED:
                Serial.println(" stopped\n");
                break;
            case SPRITE_BOUNCE:
                Serial.printf(" bounce -x:%f y:%f\n", xSpeed, ySpeed);
                break;
            case SPRITE_WRAP:
                Serial.printf(" wrap -x:%f y:%f\n", xSpeed, ySpeed);
                break;
            case SPRITE_MOVE:
                Serial.printf(" move -x:%f y:%f steps:%d\n", xSpeed, ySpeed, moveSteps);
                break;
        }
    }

    Sprite(Frame * inFrame)
    {
        frame= inFrame;
        enabled=false;
        movingState = SPRITE_STOPPED;
        colour=BLACK_COLOUR;
        moveSteps=0;
        colourSteps=0;
    }

    void enable(){
        enabled=true;
    }

    void disable()
    {
        enabled=false;
    }
    
    void setup(Colour inColour, float inBrightness, float inOpacity,
        float inX, float inY, 
        float inXSpeed, float inYSpeed, SpriteMovingState inMovingState,
        bool inEnabled)
    {
        // set the colour of the sprite using a fast fade so that it looks good
        // 10 steps means that the new colour is hit in a fifth of a second
        fadeToColour(inColour,10);
        brightness = inBrightness;
        opacity = inOpacity;
        x = inX;
        y = inY;
        xSpeed = inXSpeed;
        ySpeed = inYSpeed;
        movingState = inMovingState;
        enabled = inEnabled;
    }
};
