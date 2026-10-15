/**
 ******************************************************************************
 * @file           : pet_state.c
 * @brief          : Virtual pet state machine and stats.
 ******************************************************************************
 */

#include "pet_state.h"
#include <stdbool.h>
#include "clock.h"
#include "pet_anim.h"

/* ------------------------------------------------------------- tuning --- */
#define START_HUNGER            20U
#define START_HAPPINESS         80U
#define START_ENERGY            80U

#define HUNGER_RATE_S           5U    /* +1 hunger every N s (2x slower asleep) */
#define HAPPINESS_RATE_S        8U    /* -1 happiness every N s (none asleep)   */
#define ENERGY_RATE_S           10U   /* -1 energy every N s while awake        */
#define SLEEP_RECHARGE_S        2U    /* +1 energy every N s while asleep       */

#define HUNGRY_THRESHOLD        70U   /* above this, happiness drops faster     */
#define TIRED_THRESHOLD         20U   /* below this, happiness drops faster     */

#define FEED_AMOUNT             25U   /* hunger removed per feeding             */
#define FEED_HAPPINESS          5U    /* happiness gained per feeding           */
#define OVERFEED_PENALTY        10U   /* happiness lost if fed while full       */

#define PLAY_HAPPINESS          20U   /* happiness gained per play              */
#define PLAY_ENERGY_COST        15U   /* energy used per play (needed to play)  */
#define PLAY_HUNGER_COST        5U    /* playing makes the pet hungrier         */

#define GRUMPY_ENERGY           30U   /* woken below this energy = grumpy       */
#define GRUMPY_PENALTY          5U    /* happiness lost when woken early        */

#define FEED_DURATION_MS        2000U
#define PLAY_DURATION_MS        3000U

/* -------------------------------------------------------------- state --- */
static PetState state;
static PetStats stats;
static uint32_t state_entered_ms;
static uint32_t hunger_timer_s;
static uint32_t happiness_timer_s;
static uint32_t energy_timer_s;

static volatile PetEvent pending_event;

/* ------------------------------------------------------------ helpers --- */
static uint8_t stat_add(uint8_t value, uint8_t amount)
{
    uint32_t sum = (uint32_t)value + amount;
    return (sum > PET_STAT_MAX) ? (uint8_t)PET_STAT_MAX : (uint8_t)sum;
}

static uint8_t stat_sub(uint8_t value, uint8_t amount)
{
    return (amount >= value) ? 0U : (uint8_t)(value - amount);
}

static PetEvent take_event(void)
{
    PetEvent event = pending_event;
    pending_event = PET_EVENT_NONE;
    return event;
}

static uint32_t ms_in_state(void)
{
    return clock_millis() - state_entered_ms;
}

static void enter_state(PetState next)
{
    /* Energy switches between draining and recharging, so restart its timer
     * only when falling asleep or waking up. */
    if ((next == PET_STATE_SLEEP) != (state == PET_STATE_SLEEP))
    {
        energy_timer_s = 0;
    }

    state = next;
    state_entered_ms = clock_millis();

    /* One-time effects applied when a state is entered. */
    switch (next)
    {
    case PET_STATE_FEED:
        if (stats.hunger == 0U)
        {
            stats.happiness = stat_sub(stats.happiness, OVERFEED_PENALTY);
        }
        else
        {
            stats.hunger    = stat_sub(stats.hunger, FEED_AMOUNT);
            stats.happiness = stat_add(stats.happiness, FEED_HAPPINESS);
        }
        break;

    case PET_STATE_PLAY:
        stats.happiness = stat_add(stats.happiness, PLAY_HAPPINESS);
        stats.energy    = stat_sub(stats.energy, PLAY_ENERGY_COST);
        stats.hunger    = stat_add(stats.hunger, PLAY_HUNGER_COST);
        break;

    case PET_STATE_IDLE:
    case PET_STATE_SLEEP:
    case PET_STATE_DEAD:
    case PET_STATE_RUNAWAY:
    default:
        break;
    }

    pet_anim_start(next);
}

/* Runs once per stat tick (1 s) in every state except dead. */
static void stats_tick(void)
{
    bool asleep = (state == PET_STATE_SLEEP);

    stats.age_s++;

    /* Hunger: rises half as fast while asleep. */
    if (++hunger_timer_s >= (asleep ? 2U * HUNGER_RATE_S : HUNGER_RATE_S))
    {
        hunger_timer_s = 0;
        stats.hunger = stat_add(stats.hunger, 1U);
    }

    /* Happiness: holds steady asleep. Each of hungry / tired makes it
     * drop one step faster while awake. */
    if (!asleep)
    {
        happiness_timer_s += 1U
                           + ((stats.hunger > HUNGRY_THRESHOLD) ? 1U : 0U)
                           + ((stats.energy < TIRED_THRESHOLD)  ? 1U : 0U);
        if (happiness_timer_s >= HAPPINESS_RATE_S)
        {
            happiness_timer_s = 0;
            stats.happiness = stat_sub(stats.happiness, 1U);
        }
    }

    /* Energy: recharges asleep, drains awake. */
    if (++energy_timer_s >= (asleep ? SLEEP_RECHARGE_S : ENERGY_RATE_S))
    {
        energy_timer_s = 0;
        stats.energy = asleep ? stat_add(stats.energy, 1U)
                              : stat_sub(stats.energy, 1U);
    }
}

/*static int is_dying(void)
{
    return (stats.hunger >= PET_STAT_MAX) || (stats.happiness == 0U);
}
*/
/* ---------------------------------------------------------------- API --- */
void pet_init(void)
{
    stats.hunger      = START_HUNGER;
    stats.happiness   = START_HAPPINESS;
    stats.energy      = START_ENERGY;
    stats.age_s       = 0;
    hunger_timer_s    = 0;
    happiness_timer_s = 0;
    pending_event     = PET_EVENT_NONE;

    /* Throw away any stat ticks that piled up before (or while dead). */
    while (clock_stat_tick_pending()) { }

    enter_state(PET_STATE_IDLE);
}

void pet_request(PetEvent event)
{
    pending_event = event;
}

PetState pet_get_state(void)
{
    return state;
}

PetStats pet_get_stats(void)
{
    return stats;
}

void pet_update(void)
{
    if (state != PET_STATE_DEAD &&
    state != PET_STATE_RUNAWAY)
{
    while (clock_stat_tick_pending())
    {
        stats_tick();
    }

    /*
     * Starvation causes death.
     */
    if (stats.hunger >= PET_STAT_MAX)
    {
        enter_state(PET_STATE_DEAD);
        return;
    }

    /*
     * Zero happiness causes the pet to run away.
     */
    if (stats.happiness == 0U)
    {
        enter_state(PET_STATE_RUNAWAY);
        return;
    }
}

    PetEvent event = take_event();

    switch (state)
    {
    case PET_STATE_IDLE:
        if (stats.energy == 0U)
        {
            enter_state(PET_STATE_SLEEP);  /* collapses from exhaustion */
        }
        else if (event == PET_EVENT_FEED)
        {
            enter_state(PET_STATE_FEED);
        }
        else if (event == PET_EVENT_PLAY && stats.energy >= PLAY_ENERGY_COST)
        {
            enter_state(PET_STATE_PLAY);
        }
        else if (event == PET_EVENT_SLEEP)
        {
            enter_state(PET_STATE_SLEEP);
        }
        break;

    /* Busy: requests made during feed/play are dropped. */
    case PET_STATE_FEED:
        if (ms_in_state() >= FEED_DURATION_MS) { enter_state(PET_STATE_IDLE); }
        break;

    case PET_STATE_PLAY:
        if (ms_in_state() >= PLAY_DURATION_MS) { enter_state(PET_STATE_IDLE); }
        break;

    case PET_STATE_SLEEP:
        if (stats.energy >= PET_STAT_MAX)
        {
            enter_state(PET_STATE_IDLE);   /* wakes up on its own */
        }
        else if (event == PET_EVENT_WAKE)
        {
            if (stats.energy < GRUMPY_ENERGY)
            {
                stats.happiness = stat_sub(stats.happiness, GRUMPY_PENALTY);
            }
            enter_state(PET_STATE_IDLE);
        }
        break;

    case PET_STATE_DEAD:
    case PET_STATE_RUNAWAY:
        if (event == PET_EVENT_RESET)
        {
            pet_init();
        }
        break;

    default:
        enter_state(PET_STATE_IDLE);
        break;
    }

    pet_anim_update(state, ms_in_state());
}
