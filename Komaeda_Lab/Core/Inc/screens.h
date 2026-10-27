#ifndef SCREENS_H
#define SCREENS_H

/* UI states. Each Screen_* function runs its own loop and returns when the user leaves it. */
void Screen_Splash(void);      /* animated sprite + temperature; returns on joystick center */
void Screen_Menu(void);        /* returns when an option that leaves the menu is chosen */
void Screen_Dashboard(void);   /* joystick/DHT11 debug page; never returns */
void Wait_JoyRelease(void);

#endif
