#ifndef TILT_INPUT_H
#define TILT_INPUT_H

#include <stdint.h>
#include "snake_types.h"

typedef enum
{
    TILT_STATUS_OFF = 0,
    TILT_STATUS_CALIBRATING,
    TILT_STATUS_READY,
    TILT_STATUS_ERROR
} TiltStatus;

void TiltInput_Init(void);
void TiltInput_SetEnabled(uint8_t enabled);
uint8_t TiltInput_IsEnabled(void);
uint8_t TiltInput_IsAvailable(void);
void TiltInput_StartCalibration(void);
void TiltInput_Service(void);
Direction TiltInput_GetDirection(void);
TiltStatus TiltInput_GetStatus(void);
uint8_t TiltInput_GetWhoAmI(void);
int16_t TiltInput_GetFilteredX(void);
int16_t TiltInput_GetFilteredY(void);

#endif
