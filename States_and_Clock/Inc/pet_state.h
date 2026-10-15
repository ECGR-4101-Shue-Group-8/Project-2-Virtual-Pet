/**
 ******************************************************************************
 * @file           : pet_state.h
 * @brief          : Virtual pet state machine and stats.
 *
 * Usage:
 *   pet_init();                      once, after clock_init()
 *   pet_update();                    every pass of the main loop
 *   pet_request(PET_EVENT_FEED);     from button / sensor code (ISR-safe)
 *   pet_get_state(), pet_get_stats() from display code
 *
 * Stats are 0..PET_STAT_MAX:
 *   hunger:    0 = full,      PET_STAT_MAX = starving (pet dies)
 *   happiness: 0 = miserable (pet dies), PET_STAT_MAX = very happy
 *   energy:    0 = exhausted (pet falls asleep), PET_STAT_MAX = fully rested
 ******************************************************************************
 */

#ifndef PET_STATE_H
#define PET_STATE_H

#include <stdint.h>

#define PET_STAT_MAX 100U

typedef enum
{
    PET_STATE_IDLE = 0,
    PET_STATE_FEED,
    PET_STATE_PLAY,
    PET_STATE_SLEEP,
    PET_STATE_DEAD,
    PET_STATE_RUNAWAY,
    PET_STATE_COUNT
} PetState;

typedef enum
{
    PET_EVENT_NONE = 0,
    PET_EVENT_FEED,
    PET_EVENT_PLAY,       /* ignored if the pet is too tired */
    PET_EVENT_SLEEP,      /* e.g. lights off */
    PET_EVENT_WAKE,       /* e.g. lights on */
    PET_EVENT_RESET       /* revive / start a new pet from the dead state */
} PetEvent;

typedef struct
{
    uint8_t  hunger;
    uint8_t  happiness;
    uint8_t  energy;
    uint32_t age_s;       /* seconds alive */
} PetStats;

void     pet_init(void);
void     pet_update(void);
void     pet_request(PetEvent event);
PetState pet_get_state(void);
PetStats pet_get_stats(void);

#endif /* PET_STATE_H */
