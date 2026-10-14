#include "pet_anim.h"
#include "pet_state.h"
#include "sprites.h"
#include "display.h"

#include <stdint.h>


/* -------------------------------------------------- layout --- */

/*
 * LCD is 240 x 320.
 *
 * Our displayed monkey sprite is 128 x 128.
 * x = (240 - 128) / 2 = 56
 *
 * y = 86 gives us room above for status bars
 * and below for the menu.
 */
#define PET_X               56U
#define PET_Y               86U


/* ------------------------------------------ animation timing --- */

/*
 * pet_anim.c calls the animation function every 100 ms.
 *
 * Idle:
 *      hold each artwork frame for 400 ms
 *
 * Actions:
 *      hold each artwork frame for 500 ms
 */
#define IDLE_HOLD_TICKS     4U
#define ACTION_HOLD_TICKS   5U


/*
 * Converts the frame counter from pet_anim.c into
 * one of the actual artwork frame numbers.
 *
 * Example with 4 frames and hold_ticks = 4:
 *
 * incoming frame:  0 1 2 3 4 5 6 7 ...
 * sprite frame:    0 0 0 0 1 1 1 1 ...
 */
static uint32_t sprite_frame(
    uint32_t frame,
    uint32_t hold_ticks,
    uint32_t frame_count
)
{
    if (frame_count == 0U)
    {
        return 0U;
    }

    return (frame / hold_ticks) % frame_count;
}


/* ------------------------------------------------ drawing --- */

static void draw_pet_sprite(PetState state, uint32_t frame)
{
    const SpriteSheet *sheet = 0;

    /*
     * Select the correct sprite sheet for the current pet state.
     */
    switch (state)
    {
    case PET_STATE_IDLE:
        sheet = &sprite_idle;
        break;

    case PET_STATE_FEED:
        sheet = &sprite_feed;
        break;

    case PET_STATE_ACTION_A:
        sheet = &sprite_action_a;
        break;

    case PET_STATE_ACTION_B:
        sheet = &sprite_action_b;
        break;

    case PET_STATE_DEAD:
        sheet = &sprite_dead;
        break;

    case PET_STATE_RUNAWAY:
        sheet = &sprite_runaway;
        break;

    default:
        return;
    }


    /*
     * Don't try to draw until real RGB565 pixel data
     * has been attached to this sprite.
     */
    if (sheet->pixels == 0)
    {
        return;
    }


    /*
     * Protect against requesting a frame that does not exist.
     */
    if (sheet->frame_count != 0U)
    {
        frame %= sheet->frame_count;
    }


    display_draw_sprite_frame(
        sheet,
        (uint8_t)frame,
        PET_X,
        PET_Y
    );
}


/*
 * Later this function will draw:
 *
 *      Hunger bar
 *      Happiness bar
 *      Menu
 *      Selected action
 *
 * For now we can already retrieve the current stats.
 */
static void draw_status_ui(void)
{
    PetStats stats = pet_get_stats();

    /*
     * These suppress unused-variable warnings until
     * we add the real LCD drawing functions.
     */
    (void)stats;
}


/* -------------------------------------------- animations --- */

void anim_idle(uint32_t frame)
{
    uint32_t image = sprite_frame(
        frame,
        IDLE_HOLD_TICKS,
        sprite_idle.frame_count
    );

    draw_pet_sprite(PET_STATE_IDLE, image);
    draw_status_ui();
}


void anim_feed(uint32_t frame)
{
    uint32_t image = sprite_frame(
        frame,
        ACTION_HOLD_TICKS,
        sprite_feed.frame_count
    );

    draw_pet_sprite(PET_STATE_FEED, image);
    draw_status_ui();
}


void anim_action_a(uint32_t frame)
{
    uint32_t image = sprite_frame(
        frame,
        ACTION_HOLD_TICKS,
        sprite_action_a.frame_count
    );

    draw_pet_sprite(PET_STATE_ACTION_A, image);
    draw_status_ui();
}


void anim_action_b(uint32_t frame)
{
    uint32_t image = sprite_frame(
        frame,
        ACTION_HOLD_TICKS,
        sprite_action_b.frame_count
    );

    draw_pet_sprite(PET_STATE_ACTION_B, image);
    draw_status_ui();
}


void anim_dead(uint32_t frame)
{
    (void)frame;

    /*
     * Dead is currently one static frame.
     */
    draw_pet_sprite(PET_STATE_DEAD, 0U);
    draw_status_ui();
}


void anim_runaway(uint32_t frame)
{
    uint32_t image = sprite_frame(
        frame,
        ACTION_HOLD_TICKS,
        sprite_runaway.frame_count
    );

    draw_pet_sprite(PET_STATE_RUNAWAY, image);
    draw_status_ui();
}
