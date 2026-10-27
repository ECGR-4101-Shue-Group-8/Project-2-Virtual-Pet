#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "main.h"
#include <stdint.h>

/* Joystick (B1) bit flags returned by Joystick_Read() */
#define JOY_LEFT_MASK   (1U << 0)
#define JOY_CENTER_MASK (1U << 1)
#define JOY_DOWN_MASK   (1U << 2)
#define JOY_RIGHT_MASK  (1U << 3)
#define JOY_UP_MASK     (1U << 4)

/* Sound sensor digital output (DO) = PF15 (Arduino D2). Power the module from 5 V; PF15 is 5 V tolerant.
 * Leave AO unconnected. Adjust the module's potentiometer to set the trigger threshold. */
#define SOUND_PORT          GPIOF
#define SOUND_PIN           GPIO_PIN_15
#define SOUND_ACTIVE_LEVEL  GPIO_PIN_SET   /* change to GPIO_PIN_RESET if your module is active low */

/* Joystick */
uint8_t Joystick_Read(void);

/* Grove speaker (PE0 / TIM16). Speaker_Update() is called every 1 ms from SysTick_Handler. */
void Speaker_Init(void);
void Speaker_Update(void);

/* Sound sensor */
void SoundSensor_Init(void);
uint8_t SoundSensor_Active(void);   /* 1 while DO is at its active level */

/* DHT11: Read returns 0 on success, nonzero on a timeout or checksum error */
void DHT11_Init(void);
int DHT11_Read(uint8_t *humidity, uint8_t *temperature);

#endif
