/**
 ******************************************************************************
 * @file           : pet_anim.h
 * @brief          : Connects pet states to LCD animations.
 *
 * Each state has one animation function: void fn(uint32_t frame)
 *   frame == 0  the state was just entered (clear screen / draw first frame)
 *   frame  > 0  draw the next frame; called every PET_ANIM_FRAME_MS
 *
 * The state machine calls these automatically. To hook up the real LCD
 * animations, change the table in pet_anim.c (or define functions with the
 * same names as the placeholders, which override them).
 *
 * Animation functions should draw one frame and return quickly. Use
 * pet_get_stats() inside one if it should look different when, for example,
 * the pet is hungry.
 ******************************************************************************
 */

#ifndef PET_ANIM_H
#define PET_ANIM_H

#include <stdint.h>
#include "pet_state.h"

#define PET_ANIM_FRAME_MS 100U   /* 10 frames per second */

typedef void (*PetAnimFn)(uint32_t frame);

/* Placeholder animations (weak, do nothing). Override or replace in the table. */
void anim_idle(uint32_t frame);
void anim_feed(uint32_t frame);
void anim_action_a(uint32_t frame);
void anim_action_b(uint32_t frame);
void anim_runaway(uint32_t frame);
void anim_dead(uint32_t frame);

/* Called by pet_state.c. Not needed elsewhere. */
void pet_anim_start(PetState state);
void pet_anim_update(PetState state, uint32_t ms_in_state);

#endif /* PET_ANIM_H */
