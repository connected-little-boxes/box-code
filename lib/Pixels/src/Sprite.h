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
    SpriteMovingState stateWhenMoveCompleted;

	Frame* frame;
    Leds* leds;
	Colour colour;

    float redStep, blueStep, greenStep;
    Colour targetColour;
    int colourSteps;

	float brightness;
    float targetBrightness;
    float brightnessStep;
    int brightnessSteps;

	float opacity;

	float x;
	float y;
    float xSpeed;
    float ySpeed;

    // we can make the pixels move to a position and then start an action
    // these are the position update values
    float positionXSpeed;
    float positionYSpeed;
    float destX;
    float destY;
    int moveSteps;

    bool enabled;

    #define CLOSE_TOLERANCE 0.0001

    bool close_to(float a, float b)
    {
        float diff = a - b;
        if (diff > 0)
        {
            if (diff > CLOSE_TOLERANCE)
                return false;
            else
                return true;
        }
        else
        {
            if (diff < -CLOSE_TOLERANCE)
                return false;
            else
                return true;
        }
    }

    boolean coloursEqual(Colour a, Colour b)
    {
        if (!close_to(a.Red,b.Red)) return false;
        if (!close_to(a.Green,b.Green)) return false;
        if (!close_to(a.Blue,b.Blue)) return false;
        return true;
    }

    void setColour(Colour target){
        redStep = 0;
        blueStep = 0;
        greenStep = 0;
        colour=target;
        targetColour = target;
        colourSteps = 0;
    }

    void fadeToColour(Colour target, int noOfSteps){
        redStep = (target.Red - colour.Red) / noOfSteps;
        blueStep = (target.Blue - colour.Blue) / noOfSteps;
        greenStep = (target.Green - colour.Green) / noOfSteps;
        targetColour = target;
        colourSteps = noOfSteps;
    }

    // fade to brightness level. If we fade to black
    // the sprite is then disabled

    void fadeToBrightness(float target, int noOfSteps){
        brightnessStep = (target - brightness)/noOfSteps;
        targetBrightness = target;
        brightnessSteps = noOfSteps;
    }

    void moveToPosition(float targetX, float targetY, int noOfSteps, SpriteMovingState inStateWhenMoveCompleted)
    {
        destX=targetX ;
        destY=targetY;

        positionXSpeed = (targetX-x) / noOfSteps;
        positionYSpeed = (targetY-y) / noOfSteps;   

        stateWhenMoveCompleted = inStateWhenMoveCompleted;
        moveSteps = noOfSteps;
        movingState = SPRITE_MOVE;
    }

    void update()
    {
        if(!enabled)
            return;

        if(brightnessSteps>0)
        {
            brightness = brightness + brightnessStep;
            brightnessSteps--;
            if(brightnessSteps==0)
            {
                brightness = targetBrightness;
                if(targetBrightness==0){
                    enabled=false;
                }
            }
        }

        if(colourSteps>0)
        {
            colour.Red = colour.Red + redStep;
            colour.Blue = colour.Blue + blueStep;
            colour.Green = colour.Green + greenStep;
            colourSteps--;
            if(colourSteps==0){
                colour=targetColour;
            }
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
    
    void renderColour(Colour col);

    void bounce();

    void wrap();

    void move();

    void dump() {
        if(!enabled)
        {
            return;            
        }

        Serial.printf("r:%f g:%f b:%f bright:%f opacity:%f x:%f y:%f moveSteps:%d colourSteps:%d ",
            colour.Red, colour.Green, colour.Blue,
            brightness, opacity,
            x, y, moveSteps, colourSteps);

        switch(movingState)
        {
            case SPRITE_STOPPED:
                Serial.println(" stopped\n");
                break;
            case SPRITE_BOUNCE:
                Serial.printf(" bounce -xSpeed:%f ySpeed:%f\n", xSpeed, ySpeed);
                break;
            case SPRITE_WRAP:
                Serial.printf(" wrap -xSpeed:%f ySpeed:%f\n", xSpeed, ySpeed);
                break;
            case SPRITE_MOVE:
                Serial.printf(" move -xSpeed:%f ySpeed:%f \n", xSpeed, ySpeed );
                break;
        }
    }

    void reset()
    {
        enabled=false;
        movingState = SPRITE_STOPPED;
        colour=BLACK_COLOUR;
        moveSteps=0;
        colourSteps=0;
        brightnessSteps=0;
        x=0;
        y=0;
        xSpeed=0;
        ySpeed=0;
        redStep=0;
        greenStep=0;
        blueStep=0;
        brightnessStep=0;
    }

    Sprite(Frame * inFrame)
    {
        frame= inFrame;
        reset();
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
