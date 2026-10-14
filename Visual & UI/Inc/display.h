#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "sprites.h"

/*
 * Draw one frame from a sprite sheet.
 *
 * sheet = sprite sheet to draw from
 * frame = frame number within that sheet
 * x, y  = top-left position on LCD
 */
void display_draw_sprite_frame(
    const SpriteSheet *sheet,
    uint8_t frame,
    uint16_t x,
    uint16_t y
);

#endif /* DISPLAY_H */
