#include "stm32f401xe.h"
#include "../src/hfiles/temp_sensor.h"
#include "../src/hfiles/timer_delay.h"

#define ONEWIRE_PIN             1U
#define TEMP_READ_PERIOD_MS  2000U
#define TEMP_CONVERT_MS       750U
#define TEMP_VALID_MIN_X10     50
#define TEMP_VALID_MAX_X10   1250

typedef enum
{
    TEMP_IDLE = 0,
    TEMP_WAIT_CONVERSION
} TempState;

static uint8_t temp_has_reading = 0U;
static uint8_t temp_error = 0U;
static uint8_t temp_enabled = 0U;
static uint8_t temp_family = 0U;
static int temp_x10 = 0;
static uint32_t next_action_ms = 0U;
static TempState temp_state = TEMP_IDLE;

static void ow_low(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (ONEWIRE_PIN * 2U));
    GPIOA->MODER |=  (1UL << (ONEWIRE_PIN * 2U));
    GPIOA->OTYPER &= ~(1UL << ONEWIRE_PIN);
    GPIOA->PUPDR &= ~(3UL << (ONEWIRE_PIN * 2U));
    GPIOA->ODR &= ~(1UL << ONEWIRE_PIN);
}

static void ow_release(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (ONEWIRE_PIN * 2U));
    GPIOA->PUPDR &= ~(3UL << (ONEWIRE_PIN * 2U));
    GPIOA->PUPDR |=  (1UL << (ONEWIRE_PIN * 2U));
}

static uint8_t ow_read_pin(void)
{
    ow_release();
    return (uint8_t)((GPIOA->IDR & (1UL << ONEWIRE_PIN)) ? 1U : 0U);
}

static uint8_t ow_reset(void)
{
    uint8_t presence;

    ow_low();
    delayUs(480U);
    ow_release();
    delayUs(70U);
    presence = (uint8_t)(ow_read_pin() == 0U);
    delayUs(410U);

    return presence;
}

static void ow_write_bit(uint8_t bit)
{
    ow_low();
    if(bit)
    {
        delayUs(6U);
        ow_release();
        delayUs(64U);
    }
    else
    {
        delayUs(60U);
        ow_release();
        delayUs(10U);
    }
}

static uint8_t ow_read_bit(void)
{
    uint8_t bit;

    ow_low();
    delayUs(6U);
    ow_release();
    delayUs(9U);
    bit = ow_read_pin();
    delayUs(55U);

    return bit;
}

static void ow_write_byte(uint8_t value)
{
    uint8_t i;

    for(i = 0U; i < 8U; i++)
    {
        ow_write_bit((uint8_t)(value & 0x01U));
        value >>= 1;
    }
}

static uint8_t ow_read_byte(void)
{
    uint8_t i;
    uint8_t value = 0U;

    for(i = 0U; i < 8U; i++)
    {
        value >>= 1;
        if(ow_read_bit())
        {
            value |= 0x80U;
        }
    }

    return value;
}

static uint8_t ds_read_family(void)
{
    uint8_t i;
    uint8_t family;

    if(!ow_reset())
    {
        return 0U;
    }

    ow_write_byte(0x33U);
    family = ow_read_byte();
    for(i = 0U; i < 7U; i++)
    {
        (void)ow_read_byte();
    }

    return family;
}

static uint8_t ds_start_conversion(void)
{
    if(!ow_reset())
    {
        return 0U;
    }

    ow_write_byte(0xCCU);
    ow_write_byte(0x44U);
    return 1U;
}

static uint8_t ds_read_temp_x10(int *value_x10)
{
    uint8_t lsb;
    uint8_t msb;
    int raw;

    if(!ow_reset())
    {
        return 0U;
    }

    ow_write_byte(0xCCU);
    ow_write_byte(0xBEU);
    lsb = ow_read_byte();
    msb = ow_read_byte();
    raw = ((int)((int8_t)msb) << 8) | lsb;

    if(temp_family == 0x10U)
    {
        *value_x10 = raw * 5;
    }
    else
    {
        *value_x10 = (raw * 10) / 16;
    }

    return 1U;
}

static uint8_t temp_is_valid(int value_x10)
{
    return (uint8_t)((value_x10 >= TEMP_VALID_MIN_X10) &&
                     (value_x10 <= TEMP_VALID_MAX_X10));
}

static void temp_set_error(void)
{
    temp_has_reading = 0U;
    temp_error = 1U;
    temp_state = TEMP_IDLE;
    next_action_ms = msTicks + TEMP_READ_PERIOD_MS;
}

void TempSensor_Init(void)
{
    temp_has_reading = 0U;
    temp_error = 0U;
    temp_enabled = 0U;
    temp_family = 0U;
    temp_x10 = 0;
    temp_state = TEMP_IDLE;
    next_action_ms = msTicks + 500U;
}

void TempSensor_Enable(void)
{
    temp_enabled = 1U;
    temp_error = 0U;
    temp_state = TEMP_IDLE;
    next_action_ms = msTicks + 50U;
    ow_release();
}

void TempSensor_Service(void)
{
    int new_temp_x10;

    if(temp_enabled == 0U)
    {
        return;
    }

    if((int32_t)(msTicks - next_action_ms) < 0)
    {
        return;
    }

    if(temp_state == TEMP_IDLE)
    {
        temp_family = ds_read_family();
        if((temp_family != 0x10U) && (temp_family != 0x28U))
        {
            temp_set_error();
            return;
        }

        if(!ds_start_conversion())
        {
            temp_set_error();
            return;
        }

        temp_state = TEMP_WAIT_CONVERSION;
        next_action_ms = msTicks + TEMP_CONVERT_MS;
        return;
    }

    if(!ds_read_temp_x10(&new_temp_x10) || !temp_is_valid(new_temp_x10))
    {
        temp_set_error();
        return;
    }

    temp_x10 = new_temp_x10;
    temp_has_reading = 1U;
    temp_error = 0U;
    temp_state = TEMP_IDLE;
    next_action_ms = msTicks + TEMP_READ_PERIOD_MS;
}

uint8_t TempSensor_HasReading(void)
{
    return temp_has_reading;
}

uint8_t TempSensor_HasError(void)
{
    return temp_error;
}

int TempSensor_GetTempX10(void)
{
    return temp_x10;
}
