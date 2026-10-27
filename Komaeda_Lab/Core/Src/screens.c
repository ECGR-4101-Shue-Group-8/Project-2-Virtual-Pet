#include "screens.h"
#include "peripherals.h"
#include "ili9341.h"
#include "image_data.h"
#include <stdio.h>
#include <stdlib.h>

#define MENU_ITEM_COUNT 5U

/* Set by the menu, consumed by the splash screen */
static uint8_t debug_force_90;

void Wait_JoyRelease(void)
{
  while (Joystick_Read() != 0U) { HAL_Delay(10); }
  HAL_Delay(30);
}

void Screen_Splash(void)
{
  const uint16_t max_x = ILI9341_WIDTH - IMAGE_WIDTH;
  const uint16_t y0 = 70U;
  const uint16_t WALK_STEP = 8U;   /* pixels moved per walk frame */
  const unsigned HOT_THRESHOLD_F = 80U;
  static uint16_t sprite_x = (ILI9341_WIDTH - IMAGE_WIDTH) / 2U;   /* remembered between splash visits */

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  uint32_t frame = 0, next_tick = HAL_GetTick(), last_dht = HAL_GetTick() - 2000U;
  uint32_t anim = ANIM_IDLE, walk_loops = 0;
  uint8_t hot = 0;
  uint32_t happy_until = 0;
  const uint32_t HAPPY_HOLD_MS = 1500U;   /* keeps the animation visible after a short sound pulse */
  srand(HAL_GetTick() ^ SysTick->VAL);
  uint32_t walk_at = HAL_GetTick() + 3000U + (uint32_t)(rand() % 6000);
  while ((Joystick_Read() & JOY_CENTER_MASK) == 0U)
  {
    /* Sound sensor: happy animation while DO is high (plus a short hold) */
    if (SoundSensor_Active())
    {
      happy_until = HAL_GetTick() + HAPPY_HOLD_MS;
      if (anim != ANIM_HAPPY)
      {
        anim = ANIM_HAPPY;
        frame = 0;
        next_tick = HAL_GetTick();
      }
    }
    else if (anim == ANIM_HAPPY && (int32_t)(HAL_GetTick() - happy_until) >= 0)
    {
      anim = hot ? ANIM_HOT : ANIM_IDLE;
      frame = 0;
      next_tick = HAL_GetTick();
      walk_at = HAL_GetTick() + 3000U + (uint32_t)(rand() % 6000);
    }
    /* DHT11 allows at most one read per second; read every 2 s. The debug option
       replaces one reading with 90 F, which stays until the next real reading. */
    if (debug_force_90 || (HAL_GetTick() - last_dht) >= 2000U)
    {
      uint8_t hum, temp;
      char line[24];
      unsigned temp_f = 0;
      int ok;
      last_dht = HAL_GetTick();
      if (debug_force_90)
      {
        debug_force_90 = 0;
        temp_f = 90U;
        ok = 1;
      }
      else
      {
        ok = (DHT11_Read(&hum, &temp) == 0);
        if (ok) { temp_f = (unsigned)(temp * 9U / 5U + 32U); }
      }
      if (ok)
      {
        snprintf(line, sizeof(line), "TEMP: %u F      ", temp_f);
        hot = (temp_f >= HOT_THRESHOLD_F);
        if (anim == ANIM_HAPPY)
        {
          /* happy takes priority; the correct animation is restored when it ends */
        }
        else if (temp_f >= HOT_THRESHOLD_F && anim != ANIM_HOT)
        {
          anim = ANIM_HOT;
          frame = 0;
          next_tick = HAL_GetTick();
        }
        else if (temp_f < HOT_THRESHOLD_F && anim == ANIM_HOT)
        {
          anim = ANIM_IDLE;
          frame = 0;
          next_tick = HAL_GetTick();
          walk_at = HAL_GetTick() + 3000U + (uint32_t)(rand() % 6000);
        }
      }
      else
      {
        snprintf(line, sizeof(line), "TEMP: --- F    ");
      }
      ILI9341_DrawString(10, 10, line, ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
    }
    if ((int32_t)(HAL_GetTick() - next_tick) >= 0)
    {
      if (anim == ANIM_WALK_LEFT || anim == ANIM_WALK_RIGHT)
      {
        /* Step the sprite and erase the strip it just vacated */
        uint16_t old_x = sprite_x;
        if (anim == ANIM_WALK_RIGHT)
        {
          sprite_x = (sprite_x + WALK_STEP > max_x) ? max_x : (uint16_t)(sprite_x + WALK_STEP);
          if (sprite_x > old_x)
            ILI9341_FillRect(old_x, y0, (uint16_t)(sprite_x - old_x), IMAGE_HEIGHT, ILI9341_COLOR_BLACK);
        }
        else
        {
          sprite_x = (sprite_x < WALK_STEP) ? 0U : (uint16_t)(sprite_x - WALK_STEP);
          if (sprite_x < old_x)
            ILI9341_FillRect((uint16_t)(sprite_x + IMAGE_WIDTH), y0, (uint16_t)(old_x - sprite_x), IMAGE_HEIGHT, ILI9341_COLOR_BLACK);
        }
      }
      ILI9341_DrawBitmap(sprite_x, y0, IMAGE_WIDTH, IMAGE_HEIGHT, image_data[anim_start[anim] + frame]);
      next_tick = HAL_GetTick() + anim_frame_ms[anim_start[anim] + frame];
      frame++;

      if (frame >= anim_count[anim])
      {
        frame = 0;
        if (anim == ANIM_IDLE)
        {
          /* After a random idle period, walk left or right (away from a screen edge) */
          if ((int32_t)(HAL_GetTick() - walk_at) >= 0)
          {
            if (sprite_x == 0U)           { anim = ANIM_WALK_RIGHT; }
            else if (sprite_x >= max_x)   { anim = ANIM_WALK_LEFT; }
            else                          { anim = (rand() & 1) ? ANIM_WALK_LEFT : ANIM_WALK_RIGHT; }
            walk_loops = 1U + (uint32_t)(rand() % 6);   /* 1 loop = a single step (frames 1-2) */
          }
        }
        else if (anim == ANIM_HOT || anim == ANIM_HAPPY)
        {
          /* keep looping until the temperature drops / the sound stops */
        }
        else if (--walk_loops == 0U)
        {
          anim = ANIM_IDLE;
          walk_at = HAL_GetTick() + 3000U + (uint32_t)(rand() % 6000);
        }
      }
    }
    HAL_Delay(10);
  }
}

/* Returns when the "Back to splash" option is selected */
void Screen_Menu(void)
{
  static const char *const items[MENU_ITEM_COUNT] =
    { "BACK TO SPLASH", "DEBUG: FORCE 90 F", "OPTION 3", "OPTION 4", "OPTION 5" };
  uint8_t sel = 0, prev = 0xFF, prev_joy = 0xFF;

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  ILI9341_DrawString(10, 10, "MENU", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 3);

  while (1)
  {
    uint8_t joy = Joystick_Read();
    uint8_t pressed = joy & (uint8_t)~prev_joy;   /* rising edges only */
    prev_joy = joy;

    if ((pressed & JOY_UP_MASK) && sel > 0U)                      { sel--; }
    if ((pressed & JOY_DOWN_MASK) && sel < (MENU_ITEM_COUNT - 1U)) { sel++; }

    if (pressed & JOY_CENTER_MASK)
    {
      if (sel == 0U) { return; }
      if (sel == 1U) { debug_force_90 = 1; return; }   /* applied on the splash screen */
      /* Options 3-5: to be implemented */
    }

    if (sel != prev)
    {
      for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++)
      {
        char line[24];
        snprintf(line, sizeof(line), "%c %-14s", (i == sel) ? '>' : ' ', items[i]);
        ILI9341_DrawString(10, (uint16_t)(60 + i * 36), line,
                           (i == sel) ? ILI9341_COLOR_BLACK : ILI9341_COLOR_WHITE,
                           (i == sel) ? ILI9341_COLOR_YELLOW : ILI9341_COLOR_BLACK, 2);
      }
      prev = sel;
    }
    HAL_Delay(10);
  }
}

/**
  * @brief  Single dashboard page: joystick state refreshes continuously and the DHT11
  *         temperature/humidity refresh every 2 s, without ever leaving the page.
  */
void Screen_Dashboard(void)
{
  uint32_t last_dht = HAL_GetTick() - 2000U;
  char joy_line[] = "L:_ C:_ D:_ R:_ U:_";
  char line[24];
  uint8_t hum, temp;
  int rc;

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  ILI9341_DrawString(10, 10, "JOYSTICK", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(10, 120, "DHT11 SENSOR", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);

  while (1)
  {
    uint8_t joy = Joystick_Read();

    joy_line[2]  = (joy & JOY_LEFT_MASK)   ? '1' : '0';
    joy_line[6]  = (joy & JOY_CENTER_MASK) ? '1' : '0';
    joy_line[10] = (joy & JOY_DOWN_MASK)   ? '1' : '0';
    joy_line[14] = (joy & JOY_RIGHT_MASK)  ? '1' : '0';
    joy_line[18] = (joy & JOY_UP_MASK)     ? '1' : '0';
    ILI9341_DrawString(10, 40, joy_line, ILI9341_COLOR_GREEN, ILI9341_COLOR_BLACK, 2);

    /* The DHT11 can be read at most once per second; read every 2 s */
    if ((HAL_GetTick() - last_dht) >= 2000U)
    {
      last_dht = HAL_GetTick();
      rc = DHT11_Read(&hum, &temp);
      if (rc == 0)
      {
        snprintf(line, sizeof(line), "TEMP: %u C / %u F  ", temp, (unsigned)(temp * 9U / 5U + 32U));
        ILI9341_DrawString(10, 150, line, ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 2);
        snprintf(line, sizeof(line), "HUMIDITY: %u %%  ", hum);
        ILI9341_DrawString(10, 180, line, ILI9341_COLOR_CYAN, ILI9341_COLOR_BLACK, 2);
      }
      else
      {
        snprintf(line, sizeof(line), "READ ERROR %d      ", rc);
        ILI9341_DrawString(10, 150, line, ILI9341_COLOR_RED, ILI9341_COLOR_BLACK, 2);
      }
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }

    HAL_Delay(20);
  }
}
