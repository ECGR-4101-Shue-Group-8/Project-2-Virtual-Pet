/**
 ******************************************************************************
 * @file           : clock.h
 * @brief          : Pet timing: millisecond time base and 1 Hz stat timer.
 *
 * The system clock itself is set by SystemClock_Config() in main.c
 * (currently MSI -> PLL = 56.8 MHz). TIM6 reads the real bus clock at
 * startup, so the stat tick stays at STAT_TICK_HZ if SystemClock_Config()
 * is ever changed.
 *
 * Call order in main():
 *   HAL_Init();
 *   SystemClock_Config();
 *   clock_init();          <- after SystemClock_Config()
 ******************************************************************************
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include <stdbool.h>

#define STAT_TICK_HZ    1U          /* how often TIM6 fires a stat tick */

/* Start TIM6 as the pet stat timer. */
void clock_init(void);

/* Milliseconds since boot (HAL SysTick). Wraps after ~49 days. */
uint32_t clock_millis(void);

/* Returns true once for every stat tick that has fired since the last call. */
bool clock_stat_tick_pending(void);

#endif /* CLOCK_H */
