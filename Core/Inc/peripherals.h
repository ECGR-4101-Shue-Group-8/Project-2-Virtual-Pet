#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "main.h"
#include <stdint.h>

/* Sound sensor digital output (DO) = PF15 (Arduino D2). Power the module from 5 V; PF15 is 5 V tolerant.
 * Leave AO unconnected. Adjust the module's potentiometer to set the trigger threshold. */
#define SOUND_PORT          GPIOF
#define SOUND_PIN           GPIO_PIN_15
#define SOUND_ACTIVE_LEVEL  GPIO_PIN_SET   /* change to GPIO_PIN_RESET if your module is active low */

/* Call once after MX_GPIO_Init() and SystemClock_Config(). */
void Peripherals_Init(void);

/* Grove speaker (PE0 / TIM16). Speaker_Update() is called every 1 ms from SysTick_Handler. */
void Speaker_Update(void);
void Speaker_PlayChime(void);      /* two-note happy chime, non-blocking */

/* Sound sensor */
uint8_t SoundSensor_Active(void);  /* 1 while DO is at its active level */

/* DHT11 (data = PF13): returns 0 on success, nonzero on a timeout or checksum error */
int DHT11_Read(uint8_t *humidity, uint8_t *temperature);

#endif /* PERIPHERALS_H */
