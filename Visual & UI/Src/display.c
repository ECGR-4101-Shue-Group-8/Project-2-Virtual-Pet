#include "display.h"
#include "ili9341.h"

void display_draw_sprite_frame(
    const SpriteSheet *sheet,
    uint8_t frame,
    uint16_t x,
    uint16_t y
)
{
    if (sheet == NULL || sheet->pixels == NULL)
    {
        return;
    }

    if (frame >= sheet->frame_count)
    {
        return;
    }

    uint32_t pixels_per_frame =
        (uint32_t)sheet->frame_width *
        sheet->frame_height;

    const uint16_t *frame_pixels =
        sheet->pixels +
        ((uint32_t)frame * pixels_per_frame);

    ILI9341_DrawRGB565Bitmap(
        x,
        y,
        width,
        height,
        frame_pixels
    );
}
