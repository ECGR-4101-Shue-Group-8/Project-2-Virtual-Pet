#ifndef UI_H
#define UI_H

#include <stdint.h>

/* Initialize menu state. */
void ui_init(void);

/*
 * Process a newly pressed joystick direction.
 *
 * Bit assignments match the instructor's
 * Joystick_Read() function in main.c.
 */
void ui_joystick_event(uint8_t joystick);

#endif
