#include "display.h"
#include <stdint.h>


void display_draw_sprite_frame(
    const SpriteSheet *sheet,
    uint8_t frame,
    uint16_t x,
    uint16_t y
)
{
    /*
     * Make sure we were given a valid sprite sheet.
     */
    if (sheet == 0 || sheet->pixels == 0)
    {
        return;
    }

    /*
     * Make sure the requested frame exists.
     */
    if (frame >= sheet->frame_count)
    {
        return;
    }

    /*
     * Number of pixels contained in ONE frame.
     *
     * For a 64x64 sprite:
     *
     * 64 * 64 = 4096 pixels
     */
    uint32_t pixels_per_frame =
        (uint32_t)sheet->frame_width *
        (uint32_t)sheet->frame_height;


    /*
     * Find the beginning of the requested frame.
     *
     * Frame 0 starts at:
     *      pixels + 0
     *
     * Frame 1 starts at:
     *      pixels + 4096
     *
     * Frame 2 starts at:
     *      pixels + 8192
     *
     * etc.
     */
    const uint16_t *frame_pixels =
        sheet->pixels +
        ((uint32_t)frame * pixels_per_frame);


    /*
     * These will be used by the actual LCD drawing code.
     */
    uint16_t width  = sheet->frame_width;
    uint16_t height = sheet->frame_height;


    /*
     * TODO:
     *
     * Call the actual ST / GFX01M2 LCD drawing function here.
     *
     * It will eventually use:
     *
     *      frame_pixels
     *      width
     *      height
     *      x
     *      y
     *
     * Example concept only:
     *
     * lcd_draw_rgb565(
     *     x,
     *     y,
     *     width,
     *     height,
     *     frame_pixels
     * );
     */


    /*
     * Suppress unused-variable warnings until the
     * real LCD driver is connected.
     */
    (void)frame_pixels;
    (void)width;
    (void)height;
    (void)x;
    (void)y;
}
