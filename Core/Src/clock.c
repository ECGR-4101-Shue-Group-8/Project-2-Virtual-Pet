/**
 ******************************************************************************
 * @file           : clock.c
 * @brief          : Pet timing: millisecond time base and 1 Hz stat timer.
 *
 * TIM6 is set up with direct register access so the HAL TIM module does not
 * need to be enabled in CubeMX.
 ******************************************************************************
 */

#include "clock.h"
#include "main.h"

#define STAT_TIMER_COUNT_HZ 10000U  /* TIM6 counter rate after the prescaler */

static volatile uint32_t stat_ticks_pending;

/* TIM6 runs at PCLK1, or 2x PCLK1 when the APB1 prescaler is not /1. */
static uint32_t tim6_clock_hz(void)
{
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    return ((RCC->CFGR & RCC_CFGR_PPRE1) < RCC_CFGR_PPRE1_2) ? pclk1 : (2U * pclk1);
}

void clock_init(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* e.g. 56.8 MHz / 5680 = 10 kHz counter, / 10000 = 1 Hz update */
    TIM6->PSC = (tim6_clock_hz() / STAT_TIMER_COUNT_HZ) - 1U;
    TIM6->ARR = (STAT_TIMER_COUNT_HZ / STAT_TICK_HZ) - 1U;

    /* Load PSC immediately, then clear the update flag that UG sets so the
     * first interrupt doesn't fire early. */
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR  = 0;

    TIM6->DIER |= TIM_DIER_UIE;
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    TIM6->CR1 |= TIM_CR1_CEN;
}

uint32_t clock_millis(void)
{
    return HAL_GetTick();
}

bool clock_stat_tick_pending(void)
{
    bool pending = false;

    __disable_irq();
    if (stat_ticks_pending > 0U)
    {
        stat_ticks_pending--;
        pending = true;
    }
    __enable_irq();

    return pending;
}

void TIM6_DAC_IRQHandler(void)
{
    if (TIM6->SR & TIM_SR_UIF)
    {
        TIM6->SR = ~TIM_SR_UIF;  /* rc_w0: write 0 to clear */
        stat_ticks_pending++;
    }
}
