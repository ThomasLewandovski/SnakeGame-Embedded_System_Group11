#include "../src/hfiles/tilt_input.h"
#include "../src/hfiles/mpu6500.h"

#define TILT_EXPECTED_WHO_AM_I_A 0x70U
#define TILT_EXPECTED_WHO_AM_I_B 0x71U
#define TILT_EXPECTED_WHO_AM_I_C 0x68U
#define TILT_CALIBRATION_SAMPLES 64U
#define TILT_DEADZONE_RAW       4500
#define TILT_RELEASE_RAW        2800
#define TILT_FILTER_SHIFT       2U

static uint8_t tilt_enabled = 0U;
static uint8_t tilt_available = 0U;
static uint8_t tilt_who_am_i = 0U;
static TiltStatus tilt_status = TILT_STATUS_OFF;

static int32_t offset_x = 0;
static int32_t offset_y = 0;
static int32_t filtered_x = 0;
static int32_t filtered_y = 0;
static int32_t calibration_sum_x = 0;
static int32_t calibration_sum_y = 0;
static uint8_t calibration_count = 0U;
static Direction current_direction = DIR_NONE;

static int32_t Tilt_Abs(int32_t value)
{
    return (value < 0) ? -value : value;
}

void TiltInput_Init(void)
{
    tilt_enabled = 0U;
    tilt_available = 0U;
    tilt_who_am_i = 0U;
    tilt_status = TILT_STATUS_OFF;
    current_direction = DIR_NONE;
}

void TiltInput_SetEnabled(uint8_t enabled)
{
    tilt_enabled = (enabled != 0U) ? 1U : 0U;

    if(tilt_enabled == 0U)
    {
        tilt_status = TILT_STATUS_OFF;
        current_direction = DIR_NONE;
        return;
    }

    MPU6500_Init();
    tilt_who_am_i = MPU6500_ReadWhoAmI();
    tilt_available = (uint8_t)((tilt_who_am_i == TILT_EXPECTED_WHO_AM_I_A) ||
                               (tilt_who_am_i == TILT_EXPECTED_WHO_AM_I_B) ||
                               (tilt_who_am_i == TILT_EXPECTED_WHO_AM_I_C));

    if(tilt_available == 0U)
    {
        tilt_status = TILT_STATUS_ERROR;
        current_direction = DIR_NONE;
        return;
    }

    TiltInput_StartCalibration();
}

uint8_t TiltInput_IsEnabled(void)
{
    return tilt_enabled;
}

uint8_t TiltInput_IsAvailable(void)
{
    return tilt_available;
}

void TiltInput_StartCalibration(void)
{
    calibration_sum_x = 0;
    calibration_sum_y = 0;
    calibration_count = 0U;
    offset_x = 0;
    offset_y = 0;
    filtered_x = 0;
    filtered_y = 0;
    current_direction = DIR_NONE;
    tilt_status = tilt_available ? TILT_STATUS_CALIBRATING : TILT_STATUS_ERROR;
}

void TiltInput_Service(void)
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int32_t dx;
    int32_t dy;
    int32_t abs_x;
    int32_t abs_y;

    if((tilt_enabled == 0U) || (tilt_available == 0U))
    {
        return;
    }

    MPU6500_ReadAccelRaw(&ax, &ay, &az);
    (void)az;

    if(tilt_status == TILT_STATUS_CALIBRATING)
    {
        calibration_sum_x += ax;
        calibration_sum_y += ay;
        calibration_count++;

        if(calibration_count >= TILT_CALIBRATION_SAMPLES)
        {
            offset_x = calibration_sum_x / (int32_t)TILT_CALIBRATION_SAMPLES;
            offset_y = calibration_sum_y / (int32_t)TILT_CALIBRATION_SAMPLES;
            filtered_x = 0;
            filtered_y = 0;
            current_direction = DIR_NONE;
            tilt_status = TILT_STATUS_READY;
        }
        return;
    }

    if(tilt_status != TILT_STATUS_READY)
    {
        return;
    }

    dx = (int32_t)ax - offset_x;
    dy = (int32_t)ay - offset_y;
    filtered_x += (dx - filtered_x) >> TILT_FILTER_SHIFT;
    filtered_y += (dy - filtered_y) >> TILT_FILTER_SHIFT;

    abs_x = Tilt_Abs(filtered_x);
    abs_y = Tilt_Abs(filtered_y);

    if((abs_x < TILT_RELEASE_RAW) && (abs_y < TILT_RELEASE_RAW))
    {
        current_direction = DIR_NONE;
        return;
    }

    if((abs_x < TILT_DEADZONE_RAW) && (abs_y < TILT_DEADZONE_RAW))
    {
        return;
    }

    if(abs_x >= abs_y)
    {
        current_direction = (filtered_x > 0) ? DIR_RIGHT : DIR_LEFT;
    }
    else
    {
        current_direction = (filtered_y > 0) ? DIR_DOWN : DIR_UP;
    }
}

Direction TiltInput_GetDirection(void)
{
    return current_direction;
}

TiltStatus TiltInput_GetStatus(void)
{
    return tilt_status;
}

uint8_t TiltInput_GetWhoAmI(void)
{
    return tilt_who_am_i;
}

int16_t TiltInput_GetFilteredX(void)
{
    return (int16_t)filtered_x;
}

int16_t TiltInput_GetFilteredY(void)
{
    return (int16_t)filtered_y;
}
