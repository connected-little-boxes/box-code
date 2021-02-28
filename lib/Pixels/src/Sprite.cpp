#include "Sprite.h"
#include "Frame.h"

// declared here because of issues with forward declaration of the frame
// class - these functions use members of the Frame class can can't be 
// defined in Sprite.h

void Sprite::render()
{
    if (!enabled)
        return;

    frame->leds->renderLight(x, y, colour, brightness, opacity);
}

void Sprite::bounce()
{
    x = x + xSpeed;
    y = y + ySpeed;

    if (x < 0)
    {
        x = 0;
        xSpeed = fabs(xSpeed);
    }

    if (y < 0)
    {
        y = 0;
        ySpeed = fabs(ySpeed);
    }

    if (x >= frame->width)
    {
        x = frame->width;
        xSpeed = -fabs(xSpeed);
    }

    if (y >= frame->height)
    {
        y = frame->height;
        ySpeed = -fabs(ySpeed);
    }
}

void Sprite::wrap()
{
    x = x + xSpeed;
    y = y + ySpeed;

    if (x < 0)
    {
        x = frame->width - 1;
    }

    if (y < 0)
    {
        y = frame->height - 1;
    }

    if (x > frame->width)
    {
        x = 0;
    }

    if (y > frame->height)
    {
        y = 0;
    }
}

void Sprite::move()
{
    if(moveSteps != 0)
    {
        x = x + xSpeed;
        y = y + ySpeed;
        moveSteps--;
        if(moveSteps == 0)
        {
            movingState = SPRITE_STOPPED;
        }
    }
}


