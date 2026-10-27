#include "peripherals.h"
#include "gfx01m2_conf.h"

uint8_t Joystick_Read(void)
{
  uint8_t state = 0;

  /* All five directions are active low (see gfx01m2_conf.h) */
  if (HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port, JOY_LEFT_Pin) == GPIO_PIN_RESET)     { state |= JOY_LEFT_MASK; }
  if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port, JOY_CENTER_Pin) == GPIO_PIN_RESET) { state |= JOY_CENTER_MASK; }
  if (HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port, JOY_DOWN_Pin) == GPIO_PIN_RESET)     { state |= JOY_DOWN_MASK; }
  if (HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port, JOY_RIGHT_Pin) == GPIO_PIN_RESET)   { state |= JOY_RIGHT_MASK; }
  if (HAL_GPIO_ReadPin(JOY_UP_GPIO_Port, JOY_UP_Pin) == GPIO_PIN_RESET)         { state |= JOY_UP_MASK; }

  return state;
}

/* Grove Speaker SIG pin = PE0 driven by TIM16_CH1 (AF14).
 * Registers are used directly because the HAL TIM driver is not in this project. */
#define SPEAKER_GPIO_Port   GPIOE
#define SPEAKER_Pin         GPIO_PIN_0
#define TONE1_FREQ_HZ       196U    /* G3 */
#define TONE2_FREQ_HZ       164U    /* E3 */
#define TONE_DURATION_MS    200U    /* total; split evenly between the two notes */

static volatile uint8_t speaker_ready = 0;

static void Speaker_SetFreq(uint32_t hz)
{
  TIM16->ARR  = (HAL_RCC_GetPCLK2Freq() / (2U * hz)) - 1U;
  TIM16->CCR1 = (TIM16->ARR + 1U) / 2U;
}

void Speaker_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_TIM16_CLK_ENABLE();

  gpio.Pin = SPEAKER_Pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Alternate = GPIO_AF14_TIM16;
  HAL_GPIO_Init(SPEAKER_GPIO_Port, &gpio);

  /* 50% duty square wave at TONE_FREQ_HZ; prescale by 2 so ARR fits in 16 bits */
  TIM16->PSC  = 1U;
  Speaker_SetFreq(TONE1_FREQ_HZ);
  TIM16->CCMR1 = (TIM16->CCMR1 & ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S)) |
                 (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;  /* PWM mode 1 */
  TIM16->CCER |= TIM_CCER_CC1E;
  TIM16->BDTR |= TIM_BDTR_MOE;                                  /* TIM16 needs main output enable */
  TIM16->EGR   = TIM_EGR_UG;

  speaker_ready = 1;
}

static void Speaker_On(void)  { TIM16->CR1 |= TIM_CR1_CEN; }
static void Speaker_Off(void) { TIM16->CR1 &= ~TIM_CR1_CEN; TIM16->CNT = 0; }

static uint8_t joy_was_active = 0;
static uint32_t tone_ms_left = 0;

/* Called every 1 ms from SysTick_Handler: a new joystick press plays the tone for
 * TONE_DURATION_MS (a press during playback restarts the 500 ms). */
void Speaker_Update(void)
{
  uint8_t joy_active;

  if (!speaker_ready) { return; }

  joy_active = (Joystick_Read() != 0U);
  if (joy_active && !joy_was_active)
  {
    tone_ms_left = TONE_DURATION_MS;
    Speaker_SetFreq(TONE1_FREQ_HZ);
    TIM16->CNT = 0;
    Speaker_On();
  }
  joy_was_active = joy_active;

  if (tone_ms_left > 0U)
  {
    tone_ms_left--;
    if (tone_ms_left == 0U)
    {
      Speaker_Off();
    }
    else if (tone_ms_left == TONE_DURATION_MS / 2U)
    {
      Speaker_SetFreq(TONE2_FREQ_HZ);   /* second note */
    }
  }
}


void SoundSensor_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOF_CLK_ENABLE();
  gpio.Pin = SOUND_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SOUND_PORT, &gpio);
}

uint8_t SoundSensor_Active(void)
{
  return HAL_GPIO_ReadPin(SOUND_PORT, SOUND_PIN) == SOUND_ACTIVE_LEVEL;
}

/* DHT11 data pin = PF13 (open-drain with pull-up). Change these two defines to use another pin
 * on CN7-CN10. The DHT11 needs a ~5-10k pull-up to 3.3 V on DATA (built into most modules). */
#define DHT_PORT        GPIOF
#define DHT_PIN         GPIO_PIN_13

void DHT11_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOF_CLK_ENABLE();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);

  /* DWT cycle counter gives the microsecond timing the DHT11 protocol needs */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Waits until the line equals 'level'; returns elapsed microseconds, or -1 on timeout */
static int32_t DHT11_Wait(uint32_t level, uint32_t timeout_us)
{
  uint32_t cycles_per_us = SystemCoreClock / 1000000U;
  uint32_t start = DWT->CYCCNT;

  while (((DHT_PORT->IDR & DHT_PIN) != 0U) != (level != 0U))
  {
    if ((DWT->CYCCNT - start) > (timeout_us * cycles_per_us)) { return -1; }
  }
  return (int32_t)((DWT->CYCCNT - start) / cycles_per_us);
}

/**
  * @brief  Reads the DHT11. Returns 0 on success, nonzero on a timeout or checksum error.
  * @param  humidity    %RH (integer)
  * @param  temperature degrees C (integer)
  */
int DHT11_Read(uint8_t *humidity, uint8_t *temperature)
{
  uint8_t data[5] = {0};
  int32_t t;
  int i;
  int err = 0;

  /* Start signal: pull low for >18 ms, then release */
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(20);

  /* The 40-bit frame takes ~4 ms; keep interrupts (SysTick/audio) out of the timing */
  __disable_irq();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);

  if (DHT11_Wait(0, 100) < 0 || DHT11_Wait(1, 120) < 0 || DHT11_Wait(0, 120) < 0)
  {
    err = 1;
  }

  for (i = 0; (i < 40) && !err; i++)
  {
    if (DHT11_Wait(1, 100) < 0) { err = 1; break; }     /* ~50 us low gap */
    t = DHT11_Wait(0, 120);                             /* high time: ~27 us = 0, ~70 us = 1 */
    if (t < 0) { err = 1; break; }
    data[i / 8] <<= 1;
    if (t > 40) { data[i / 8] |= 1U; }
  }
  __enable_irq();

  if (err) { return 1; }
  if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) { return 2; }

  *humidity = data[0];
  *temperature = data[2];
  return 0;
}
