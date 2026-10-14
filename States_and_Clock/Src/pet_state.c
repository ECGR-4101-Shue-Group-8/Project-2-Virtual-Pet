/**
 ******************************************************************************
 * @file           : pet_state.c
 * @brief          : Virtual pet state machine and stats.
 ******************************************************************************
 */

#include "pet_state.h"
#include "clock.h"
#include "pet_anim.h"

/* ------------------------------------------------------------- tuning --- */
#define START_HUNGER            20U
#define START_HAPPINESS         80U

#define HUNGER_RATE_S           5U    /* +1 hunger every N seconds          */
#define HAPPINESS_RATE_S        8U    /* -1 happiness every N seconds       */
#define HUNGRY_THRESHOLD        70U   /* above this, happiness drops 2x     */

#define FEED_AMOUNT             25U   /* hunger removed per feeding         */
#define FEED_HAPPINESS          5U    /* happiness gained per feeding       */
#define OVERFEED_PENALTY        10U   /* happiness lost if fed while full   */

#define ACTION_A_HAPPINESS      15U   /* TODO: set once action A is defined */
#define ACTION_B_HAPPINESS      15U   /* TODO: set once action B is defined */

#define ACTION_DURATION_MS      2000U /* time spent in feed/action states   */

/* -------------------------------------------------------------- state --- */
static PetState state;
static PetStats stats;
static uint32_t state_entered_ms;
static uint32_t hunger_timer_s;
static uint32_t happiness_timer_s;

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

static void enter_state(PetState next)
{
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

    case PET_STATE_ACTION_A:
        /* TODO: start action A peripheral here */
        stats.happiness = stat_add(stats.happiness, ACTION_A_HAPPINESS);
        break;

    case PET_STATE_ACTION_B:
        /* TODO: start action B peripheral here */
        stats.happiness = stat_add(stats.happiness, ACTION_B_HAPPINESS);
        break;

    case PET_STATE_IDLE:
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
    stats.age_s++;

    if (++hunger_timer_s >= HUNGER_RATE_S)
    {
        hunger_timer_s = 0;
        stats.hunger = stat_add(stats.hunger, 1U);
    }

    /* A hungry pet gets unhappy twice as fast. */
    happiness_timer_s += (stats.hunger > HUNGRY_THRESHOLD) ? 2U : 1U;
    if (happiness_timer_s >= HAPPINESS_RATE_S)
    {
        happiness_timer_s = 0;
        stats.happiness = stat_sub(stats.happiness, 1U);
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
        if      (event == PET_EVENT_FEED)     { enter_state(PET_STATE_FEED); }
        else if (event == PET_EVENT_ACTION_A) { enter_state(PET_STATE_ACTION_A); }
        else if (event == PET_EVENT_ACTION_B) { enter_state(PET_STATE_ACTION_B); }
        break;

    case PET_STATE_FEED:
    case PET_STATE_ACTION_A:
    case PET_STATE_ACTION_B:
        /* Busy: requests made during an action are dropped. */
        if ((clock_millis() - state_entered_ms) >= ACTION_DURATION_MS)
        {
            enter_state(PET_STATE_IDLE);
        }
        break;

    case PET_STATE_DEAD:
        if (event == PET_EVENT_RESET)
        {
            pet_init();
        }
        break;

    default:
        enter_state(PET_STATE_IDLE);
        break;
    }

    pet_anim_update(state, clock_millis() - state_entered_ms);
}
