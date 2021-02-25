#include "Sprite.h"

void Sprite::render() {
    frame->leds->renderDot(x, y, oldX, oldY, colour, brightness, opacity);
}

