#include "ui.h"

#include "pet_anim.h"
#include "pet_state.h"
#include "sprites.h"
#include "display.h"
#include "ili9341.h"

#include <stdint.h>
#include <stdio.h>


/* ---------------- Joystick masks ---------------- */

#define JOY_LEFT       (1U << 0)
#define JOY_CENTER     (1U << 1)
#define JOY_DOWN       (1U << 2)
#define JOY_RIGHT      (1U << 3)
#define JOY_UP         (1U << 4)


/* ---------------- Layout ---------------- */

#define PET_X          88U
#define PET_Y          116U

#define COLOR_BG       ILI9341_COLOR_BLACK
#define COLOR_TEXT     ILI9341_COLOR_WHITE

#define MENU_COUNT     3U


/* ---------------- UI State ---------------- */

static uint8_t menu_open = 0;
static uint8_t selected_action = 0;

static uint32_t previous_step = UINT32_MAX;

static int16_t previous_hunger = -1;
static int16_t previous_happiness = -1;


static const char *menu_options[MENU_COUNT] =
{
    "FEED",
    "PLAY",
    "REST"
};


/* ---------------- Initialization ---------------- */

void ui_init(void)
{
    menu_open = 0;
    selected_action = 0;

    previous_step = UINT32_MAX;

    previous_hunger = -1;
    previous_happiness = -1;
}


/* ---------------- Statistics ---------------- */

static void draw_stats(void)
{
    PetStats stats = pet_get_stats();

    if (previous_hunger == stats.hunger &&
        previous_happiness == stats.happiness)
    {
        return;
    }

    char text[8];

    /* Hunger meter */

    ILI9341_FillRect(
        86, 50, 115, 12,
        ILI9341_COLOR_GRAY
    );

    uint16_t hunger_width =
        (uint16_t)((stats.hunger * 115U) / 100U);

    if (hunger_width > 0)
    {
        ILI9341_FillRect(
            86, 50,
            hunger_width, 12,
            ILI9341_COLOR_ORANGE
        );
    }

    snprintf(
        text,
        sizeof(text),
        "%3u",
        (unsigned)stats.hunger
    );

    ILI9341_DrawString(
        207, 52,
        text,
        COLOR_TEXT,
        COLOR_BG,
        1
    );


    /* Happiness meter */

    ILI9341_FillRect(
        86, 77, 115, 12,
        ILI9341_COLOR_GRAY
    );

    uint16_t happiness_width =
        (uint16_t)((stats.happiness * 115U) / 100U);

    if (happiness_width > 0)
    {
        ILI9341_FillRect(
            86, 77,
            happiness_width, 12,
            ILI9341_COLOR_GREEN
        );
    }

    snprintf(
        text,
        sizeof(text),
        "%3u",
        (unsigned)stats.happiness
    );

    ILI9341_DrawString(
        207, 79,
        text,
        COLOR_TEXT,
        COLOR_BG,
        1
    );

    previous_hunger = stats.hunger;
    previous_happiness = stats.happiness;
}


/* ---------------- Menu ---------------- */

static void draw_menu(PetState state)
{
    ILI9341_FillRect(
        0, 237, 240, 83,
        COLOR_BG
    );

    if (state == PET_STATE_DEAD)
    {
        ILI9341_DrawString(
            55, 250,
            "MONKEY DIED",
            ILI9341_COLOR_RED,
            COLOR_BG,
            1
        );

        ILI9341_DrawString(
            30, 278,
            "CENTER: RESTART",
            COLOR_TEXT,
            COLOR_BG,
            1
        );

        return;
    }

    if (state == PET_STATE_RUNAWAY)
    {
        ILI9341_DrawString(
            45, 250,
            "MONKEY RAN AWAY",
            ILI9341_COLOR_YELLOW,
            COLOR_BG,
            1
        );

        ILI9341_DrawString(
            30, 278,
            "CENTER: RESTART",
            COLOR_TEXT,
            COLOR_BG,
            1
        );

        return;
    }

    if (state != PET_STATE_IDLE)
    {
        const char *message = "";

        switch (state)
        {
            case PET_STATE_FEED:
                message = "FEEDING...";
                break;

            case PET_STATE_ACTION_A:
                message = "PLAYING...";
                break;

            case PET_STATE_ACTION_B:
                message = "RESTING...";
                break;

            default:
                break;
        }

        ILI9341_DrawString(
            65, 260,
            message,
            ILI9341_COLOR_CYAN,
            COLOR_BG,
            1
        );

        return;
    }

    if (!menu_open)
    {
        ILI9341_DrawString(
            35, 260,
            "CENTER: ACTIONS",
            COLOR_TEXT,
            COLOR_BG,
            1
        );

        return;
    }

    ILI9341_DrawString(
        20, 243,
        "SELECT ACTION",
        COLOR_TEXT,
        COLOR_BG,
        1
    );

    for (uint8_t i = 0; i < MENU_COUNT; i++)
    {
        uint16_t y = (uint16_t)(261 + i * 16);

        uint16_t color =
            (i == selected_action)
            ? ILI9341_COLOR_YELLOW
            : COLOR_TEXT;

        ILI9341_DrawString(
            16, y,
            (i == selected_action) ? ">" : " ",
            color,
            COLOR_BG,
            1
        );

        ILI9341_DrawString(
            34, y,
            menu_options[i],
            color,
            COLOR_BG,
            1
        );
    }
}


/* ---------------- Screen initialization ---------------- */

static void draw_main_screen(PetState state)
{
    ILI9341_FillScreen(COLOR_BG);

    ILI9341_DrawString(
        55, 15,
        "MONKEY PAL",
        COLOR_TEXT,
        COLOR_BG,
        2
    );

    ILI9341_DrawString(
        10, 52,
        "Hunger",
        COLOR_TEXT,
        COLOR_BG,
        1
    );

    ILI9341_DrawString(
        10, 79,
        "Happy",
        COLOR_TEXT,
        COLOR_BG,
        1
    );

    previous_hunger = -1;
    previous_happiness = -1;
    previous_step = UINT32_MAX;

    draw_stats();
    draw_menu(state);
}


/* ---------------- Sprite selection ---------------- */

static const SpriteSheet *get_sprite(PetState state)
{
    switch (state)
    {
        case PET_STATE_IDLE:
            return &sprite_idle;

        case PET_STATE_FEED:
            return &sprite_feed;

        case PET_STATE_ACTION_A:
            return &sprite_action_a;

        case PET_STATE_ACTION_B:
            return &sprite_action_b;

        case PET_STATE_DEAD:
            return &sprite_dead;

        case PET_STATE_RUNAWAY:
            return &sprite_runaway;

        default:
            return NULL;
    }
}


/* ---------------- Sprite animation ---------------- */

static void draw_animation(
    PetState state,
    uint32_t frame
)
{
    const SpriteSheet *sprite = get_sprite(state);

    if (sprite == NULL || sprite->frame_count == 0)
    {
        return;
    }

    uint32_t hold_ticks = 4;

    if (state == PET_STATE_FEED ||
        state == PET_STATE_RUNAWAY)
    {
        hold_ticks = 5;
    }

    uint32_t step = frame / hold_ticks;

    if (state == PET_STATE_DEAD)
    {
        step = 0;
    }

    if (step == previous_step)
    {
        return;
    }

    previous_step = step;

    uint8_t sprite_frame =
        (uint8_t)(step % sprite->frame_count);

    uint16_t x = PET_X;
    uint16_t y = PET_Y;

    /* Clear only the animation region */

    ILI9341_FillRect(
        65, 100, 115, 100,
        COLOR_BG
    );


    /* PLAY: monkey jumps left and right */

    if (state == PET_STATE_ACTION_A)
    {
        if (step % 2 == 0)
        {
            x = 80;
            y = 118;
        }
        else
        {
            x = 96;
            y = 106;
        }

        /* Moving ball */

        uint16_t ball_x =
            (step % 2 == 0) ? 155 : 75;

        ILI9341_FillRect(
            ball_x, 175,
            9, 9,
            ILI9341_COLOR_ORANGE
        );
    }


    /* REST: monkey stays low and snoozes */

    if (state == PET_STATE_ACTION_B)
    {
        y = 122;

        ILI9341_DrawString(
            (step % 2 == 0) ? 150 : 158,
            (step % 2 == 0) ? 105 : 112,
            "Zz",
            ILI9341_COLOR_CYAN,
            COLOR_BG,
            1
        );
    }


    /* RUNAWAY: monkey moves across the screen */

    if (state == PET_STATE_RUNAWAY)
    {
        x = (step % 2 == 0) ? 80 : 110;
    }


    display_draw_sprite_frame(
        sprite,
        sprite_frame,
        x,
        y
    );
}


/* ---------------- Common animation renderer ---------------- */

static void render_state(
    PetState state,
    uint32_t frame
)
{
    if (frame == 0)
    {
        draw_main_screen(state);
    }

    draw_stats();
    draw_animation(state, frame);
}


/* ---------------- State animation callbacks ---------------- */

void anim_idle(uint32_t frame)
{
    render_state(PET_STATE_IDLE, frame);
}

void anim_feed(uint32_t frame)
{
    render_state(PET_STATE_FEED, frame);
}

void anim_action_a(uint32_t frame)
{
    render_state(PET_STATE_ACTION_A, frame);
}

void anim_action_b(uint32_t frame)
{
    render_state(PET_STATE_ACTION_B, frame);
}

void anim_dead(uint32_t frame)
{
    render_state(PET_STATE_DEAD, frame);
}

void anim_runaway(uint32_t frame)
{
    render_state(PET_STATE_RUNAWAY, frame);
}


/* ---------------- Joystick handling ---------------- */

void ui_joystick_event(uint8_t joystick)
{
    PetState state = pet_get_state();

    /* Restart pet after death or running away */

    if (state == PET_STATE_DEAD ||
        state == PET_STATE_RUNAWAY)
    {
        if (joystick & JOY_CENTER)
        {
            pet_request(PET_EVENT_RESET);
        }

        return;
    }


    /* Ignore menu input during actions */

    if (state != PET_STATE_IDLE)
    {
        return;
    }


    /* Open menu */

    if (!menu_open)
    {
        if (joystick & (JOY_CENTER | JOY_RIGHT))
        {
            menu_open = 1;
            selected_action = 0;

            draw_menu(state);
        }

        return;
    }


    /* Close menu */

    if (joystick & JOY_LEFT)
    {
        menu_open = 0;
        draw_menu(state);
        return;
    }


    /* Move selection up */

    if (joystick & JOY_UP)
    {
        selected_action =
            (selected_action + MENU_COUNT - 1)
            % MENU_COUNT;

        draw_menu(state);
        return;
    }


    /* Move selection down */

    if (joystick & JOY_DOWN)
    {
        selected_action =
            (selected_action + 1)
            % MENU_COUNT;

        draw_menu(state);
        return;
    }


    /* Activate selected action */

    if (joystick & JOY_CENTER)
    {
        menu_open = 0;

        switch (selected_action)
        {
            case 0:
                pet_request(PET_EVENT_FEED);
                break;

            case 1:
                pet_request(PET_EVENT_ACTION_A);
                break;

            case 2:
                pet_request(PET_EVENT_ACTION_B);
                break;

            default:
                break;
        }
    }
}
