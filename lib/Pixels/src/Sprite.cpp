#include "Sprite.h"

// declared here becuase of issues with forward declaration of the frame
// class

void Sprite::render()
{
    if (!enabled)
        return;

    frame->leds->renderLight(x,y,colour, brightness, opacity );

//    frame->leds->renderDot(x, y, colour, brightness, opacity);
}
