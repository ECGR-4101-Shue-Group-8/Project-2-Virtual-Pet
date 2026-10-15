#include "sprites.h"

/*
 * Sprite sheet definitions.
 *
 * Pixel data has not been generated yet, so each .pixels pointer
 * is temporarily NULL (0).
 *
 * After the PNG files are converted to RGB565, these pointers
 * will point to the generated pixel arrays.
 *
 * Current intended displayed frame size:
 *      64 x 64 pixels
 *
 * The original Piskel artwork can remain 64 x 64 and be
 * upscaled 2x with nearest-neighbor before RGB565 conversion.
 */

extern const uint16_t idle_pixels[];
extern const uint16_t feed_pixels[];
//extern const uint16_t action_a_pixels[];
//extern const uint16_t action_b_pixels[];
extern const uint16_t runaway_pixels[];
extern const uint16_t dead_pixels[];

/* Idle */

const SpriteSheet sprite_idle =
{
    .pixels = idle_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 4
};


/* Feeding */

const SpriteSheet sprite_feed =
{
    .pixels = feed_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 4
};


/* Play - temporarily uses idle artwork */

const SpriteSheet sprite_action_a =
{
    .pixels = idle_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 4
};


/* Rest - temporarily uses idle artwork */

const SpriteSheet sprite_action_b =
{
    .pixels = idle_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 4
};


/* Death */

const SpriteSheet sprite_dead =
{
    .pixels = dead_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 1
};


/* Runaway */

const SpriteSheet sprite_runaway =
{
    .pixels = runaway_pixels,
    .frame_width = 64,
    .frame_height = 64,
    .frame_count = 4
};

