#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>

void TempSensor_Init(void);
void TempSensor_Service(void);
void TempSensor_Enable(void);
uint8_t TempSensor_HasReading(void);
uint8_t TempSensor_HasError(void);
int TempSensor_GetTempX10(void);

#endif
