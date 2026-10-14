#include "display.h"

/*
 * Temporary display implementation.
 *
 * This will eventually contain the actual ST/BSP LCD calls.
 * For now it does nothing so the rest of the project can
 * compile independently of the LCD driver.
 */

void display_draw_sprite_frame(
    const SpriteSheet *sheet,
    uint8_t frame,
    uint16_t x,
    uint16_t y
)
{
    (void)sheet;
    (void)frame;
    (void)x;
    (void)y;

    /*
     * TODO:
     * Replace this with the real LCD drawing code once
     * we identify the functions in the starter project.
     */
}
