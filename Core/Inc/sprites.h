#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

/*
 * Describes one sprite sheet stored as RGB565 pixel data.
 *
 * pixels       = pointer to RGB565 pixel array
 * frame_width  = width of one animation frame
 * frame_height = height of one animation frame
 * frame_count  = number of frames in the sprite sheet
 */
typedef struct
{
    const uint16_t *pixels;

    uint16_t frame_width;
    uint16_t frame_height;

    uint8_t frame_count;

} SpriteSheet;


/*
 * Sprite sheets used by the pet UI.
 *
 * The actual pixel data and definitions will be placed
 * in sprites.c after the PNG images are converted to RGB565.
 */

extern const SpriteSheet sprite_idle;
extern const SpriteSheet sprite_feed;
extern const SpriteSheet sprite_action_a;
extern const SpriteSheet sprite_action_b;
extern const SpriteSheet sprite_dead;
extern const SpriteSheet sprite_runaway;


#endif /* SPRITES_H */
