#ifndef APP_H
#define APP_H

#include <stdint.h>

void App_Init(void);
void App_Run(void);
void App_Main(void);
void App_MainWithTilt(uint8_t enable_tilt_control);

#endif
