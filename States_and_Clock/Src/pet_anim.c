/**
 ******************************************************************************
 * @file           : pet_anim.c
 * @brief          : Connects pet states to LCD animations.
 ******************************************************************************
 */

#include "pet_anim.h"

/* Which animation plays in which state. Swap in the LCD team's functions. */
static const PetAnimFn state_anims[PET_STATE_COUNT] =
{
    [PET_STATE_IDLE]     = anim_idle,
    [PET_STATE_FEED]     = anim_feed,
    [PET_STATE_ACTION_A] = anim_action_a,
    [PET_STATE_ACTION_B] = anim_action_b,
    [PET_STATE_DEAD]     = anim_dead,
};

static uint32_t current_frame;

/* Placeholders so the project builds before the LCD code exists. */
__attribute__((weak)) void anim_idle(uint32_t frame)     { (void)frame; }
__attribute__((weak)) void anim_feed(uint32_t frame)     { (void)frame; }
__attribute__((weak)) void anim_action_a(uint32_t frame) { (void)frame; }
__attribute__((weak)) void anim_action_b(uint32_t frame) { (void)frame; }
__attribute__((weak)) void anim_dead(uint32_t frame)     { (void)frame; }

static void play_frame(PetState state, uint32_t frame)
{
    if (state < PET_STATE_COUNT && state_anims[state] != 0)
    {
        state_anims[state](frame);
    }
}

void pet_anim_start(PetState state)
{
    current_frame = 0;
    play_frame(state, current_frame);
}

void pet_anim_update(PetState state, uint32_t ms_in_state)
{
    /* Frames are timed from state entry. If drawing ran long, skip ahead
     * instead of drawing every missed frame. */
    uint32_t due_frame = ms_in_state / PET_ANIM_FRAME_MS;
    if (due_frame > current_frame)
    {
        current_frame = due_frame;
        play_frame(state, current_frame);
    }
}
