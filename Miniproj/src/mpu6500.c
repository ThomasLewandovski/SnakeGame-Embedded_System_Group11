#include "../src/hfiles/mpu6500.h"
#include "../src/hfiles/i2c1_bus.h"
#include "../src/hfiles/timer_delay.h"

#define MPU6500_ACCEL_CONFIG 0x1CU
#define MPU6500_GYRO_CONFIG  0x1BU
#define MPU6500_CONFIG       0x1AU

void MPU6500_Init(void)
{
    I2C1_WriteRegister(MPU6500_ADDR, MPU6500_PWR_MGMT_1, 0x00U);
    delayMs(10U);
    I2C1_WriteRegister(MPU6500_ADDR, MPU6500_CONFIG, 0x03U);
    I2C1_WriteRegister(MPU6500_ADDR, MPU6500_ACCEL_CONFIG, 0x00U);
    I2C1_WriteRegister(MPU6500_ADDR, MPU6500_GYRO_CONFIG, 0x00U);
}

uint8_t MPU6500_ReadWhoAmI(void)
{
    return I2C1_ReadRegister(MPU6500_ADDR, MPU6500_WHO_AM_I);
}

int16_t MPU6500_Read16(uint8_t reg)
{
    uint8_t high = I2C1_ReadRegister(MPU6500_ADDR, reg);
    uint8_t low = I2C1_ReadRegister(MPU6500_ADDR, (uint8_t)(reg + 1U));

    return (int16_t)((uint16_t)((uint16_t)high << 8U) | low);
}

void MPU6500_ReadAccelRaw(int16_t *ax, int16_t *ay, int16_t *az)
{
    *ax = MPU6500_Read16(MPU6500_ACCEL_XOUT_H);
    *ay = MPU6500_Read16(MPU6500_ACCEL_YOUT_H);
    *az = MPU6500_Read16(MPU6500_ACCEL_ZOUT_H);
}

void MPU6500_ReadGyroRaw(int16_t *gx, int16_t *gy, int16_t *gz)
{
    *gx = MPU6500_Read16(MPU6500_GYRO_XOUT_H);
    *gy = MPU6500_Read16(MPU6500_GYRO_YOUT_H);
    *gz = MPU6500_Read16(MPU6500_GYRO_ZOUT_H);
}
