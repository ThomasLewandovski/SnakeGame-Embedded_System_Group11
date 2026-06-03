#include "../src/hfiles/app.h"
#include "../src/hfiles/buttons.h"
#include "../src/hfiles/i2c1_bus.h"
#include "../src/hfiles/lcd.h"
#include "../src/hfiles/menu.h"
#include "../src/hfiles/OLED.h"
#include "../src/hfiles/snake_game.h"
#include "../src/hfiles/snake_render.h"
#include "../src/hfiles/temp_sensor.h"
#include "../src/hfiles/timer_delay.h"
#include "../src/hfiles/utils.h"

typedef enum
{
    APP_MENU = 0,
    APP_GAME,
    APP_PAUSE,
    APP_GAME_OVER
} AppState;

static AppState app_state = APP_MENU;
static MenuState menu;
static SnakeGame game;
static uint16_t high_score = 0U;
static uint32_t last_step_ms = 0U;
static uint32_t last_lcd_ms = 0U;
static uint8_t previous_buttons = 0U;
static uint8_t button_latch = 0U;
static Direction queued_turn = DIR_NONE;
static uint16_t last_lcd_score = 0xFFFFU;
static int last_lcd_temp = -9999;
static uint8_t last_lcd_temp_ready = 0xFFU;
static uint8_t last_lcd_temp_error = 0xFFU;

#define BUTTON_K1_UP_MASK      0x01U
#define BUTTON_K2_DOWN_MASK    0x02U
#define BUTTON_K3_HASH_MASK    0x04U
#define BUTTON_K4_STAR_MASK    0x08U
#define APP_LOOP_DELAY_MS    1U
#define LCD_UPDATE_PERIOD_MS 250U

static uint8_t App_ReadButtons(void)
{
    uint8_t buttons = 0U;

    if(Button_UP_Pressed())
    {
        buttons |= BUTTON_K1_UP_MASK;
    }
    if(Button_Sharp_Pressed())
    {
        buttons |= BUTTON_K2_DOWN_MASK;
    }
    if(Button_DOWN_Pressed())
    {
        buttons |= BUTTON_K3_HASH_MASK;
    }
    if(Button_AS_Pressed())
    {
        buttons |= BUTTON_K4_STAR_MASK;
    }

    return buttons;
}

static uint16_t App_CurrentScore(void)
{
    if((app_state == APP_GAME) ||
       (app_state == APP_PAUSE) ||
       (app_state == APP_GAME_OVER))
    {
        return game.score;
    }

    return 0U;
}

static void App_RenderLcd(uint8_t force)
{
    char buffer[12];
    int temp_x10 = TempSensor_GetTempX10();
    int whole = temp_x10 / 10;
    int frac = temp_x10 % 10;
    uint16_t score = App_CurrentScore();
    uint8_t temp_ready = TempSensor_HasReading();
    uint8_t temp_error = TempSensor_HasError();

    if(frac < 0)
    {
        frac = -frac;
    }

    if((force == 0U) &&
       (last_lcd_score == score) &&
       (last_lcd_temp == temp_x10) &&
       (last_lcd_temp_ready == temp_ready) &&
       (last_lcd_temp_error == temp_error))
    {
        return;
    }

    lcd_set_cursor(0, 0);
    lcd_print((char *)"Score:");
    intToStr((int)score, buffer);
    lcd_print(buffer);
    lcd_print((char *)"      ");

    lcd_set_cursor(0, 1);
    lcd_print((char *)"Temp:");
    if(temp_ready)
    {
        intToStr(whole, buffer);
        lcd_print(buffer);
        lcd_print((char *)".");
        intToStr(frac, buffer);
        lcd_print(buffer);
        lcd_print((char *)"C   ");
    }
    else if(temp_error)
    {
        lcd_print((char *)"ERR    ");
    }
    else
    {
        lcd_print((char *)"--.-C   ");
    }

    last_lcd_score = score;
    last_lcd_temp = temp_x10;
    last_lcd_temp_ready = temp_ready;
    last_lcd_temp_error = temp_error;
}

static uint8_t App_ButtonPressed(uint8_t buttons, uint8_t mask)
{
    return (uint8_t)(((buttons & mask) != 0U) && ((previous_buttons & mask) == 0U));
}

static uint8_t App_ConsumeButton(uint8_t mask)
{
    if((button_latch & mask) == 0U)
    {
        return 0U;
    }

    button_latch &= (uint8_t)~mask;
    return 1U;
}

static uint8_t App_IsOppositeDirection(Direction a, Direction b)
{
    return ((a == DIR_UP) && (b == DIR_DOWN)) ||
           ((a == DIR_DOWN) && (b == DIR_UP)) ||
           ((a == DIR_LEFT) && (b == DIR_RIGHT)) ||
           ((a == DIR_RIGHT) && (b == DIR_LEFT));
}

static void App_ApplyDirection(Direction direction)
{
    Direction last_direction;

    if(direction == DIR_NONE)
    {
        return;
    }

    if(game.pending_direction == game.direction)
    {
        last_direction = game.direction;
        if((direction != last_direction) &&
           !App_IsOppositeDirection(last_direction, direction))
        {
            game.pending_direction = direction;
        }
        return;
    }

    last_direction = game.pending_direction;
    if((direction != last_direction) &&
       !App_IsOppositeDirection(last_direction, direction))
    {
        queued_turn = direction;
    }
}

static void App_ApplyDirectionInputs(uint8_t buttons)
{
    if((buttons & BUTTON_K1_UP_MASK) != 0U)
    {
        App_ApplyDirection(DIR_UP);
    }
    if((buttons & BUTTON_K2_DOWN_MASK) != 0U)
    {
        App_ApplyDirection(DIR_DOWN);
    }
    if((buttons & BUTTON_K4_STAR_MASK) != 0U)
    {
        App_ApplyDirection(DIR_LEFT);
    }
    if((buttons & BUTTON_K3_HASH_MASK) != 0U)
    {
        App_ApplyDirection(DIR_RIGHT);
    }
}

static uint8_t App_PauseCombo(uint8_t buttons)
{
    return (uint8_t)(((buttons & BUTTON_K3_HASH_MASK) != 0U) &&
                     ((buttons & BUTTON_K4_STAR_MASK) != 0U));
}

static uint8_t App_PauseComboPressed(uint8_t buttons)
{
    return (uint8_t)(App_PauseCombo(buttons) && !App_PauseCombo(previous_buttons));
}

static void App_StartGame(void)
{
    Snake_Init(&game, menu.difficulty, high_score);
    app_state = APP_GAME;
    last_step_ms = msTicks;
    button_latch = 0U;
    queued_turn = DIR_NONE;
    OLED_Clear();
    Snake_RenderInvalidate();
    Snake_RenderGame(&game);
}

static void App_HandleMenu(void)
{
    if(App_ConsumeButton(BUTTON_K1_UP_MASK))
    {
        Menu_PreviousDifficulty(&menu);
        Menu_Render(&menu, high_score);
    }
    else if(App_ConsumeButton(BUTTON_K2_DOWN_MASK))
    {
        Menu_NextDifficulty(&menu);
        Menu_Render(&menu, high_score);
    }
    else if(App_ConsumeButton(BUTTON_K3_HASH_MASK))
    {
        App_StartGame();
    }
}

static void App_HandleGame(uint8_t buttons)
{
    if(App_PauseComboPressed(buttons))
    {
        app_state = APP_PAUSE;
        game.state = GAME_PAUSED;
        button_latch = 0U;
        Snake_RenderPause(&game);
        return;
    }

    button_latch |= buttons;
    App_ApplyDirectionInputs(button_latch);
    button_latch &= (uint8_t)~(BUTTON_K1_UP_MASK |
                               BUTTON_K2_DOWN_MASK |
                               BUTTON_K3_HASH_MASK |
                               BUTTON_K4_STAR_MASK);

    if((uint32_t)(msTicks - last_step_ms) >= Snake_GetStepPeriodMs(game.difficulty))
    {
        last_step_ms = msTicks;
        Snake_Update(&game);

        if((queued_turn != DIR_NONE) &&
           !App_IsOppositeDirection(game.direction, queued_turn))
        {
            game.pending_direction = queued_turn;
            queued_turn = DIR_NONE;
        }

        if(game.state == GAME_OVER)
        {
            high_score = game.high_score;
            app_state = APP_GAME_OVER;
            button_latch = 0U;
            Snake_RenderGameOver(&game);
            return;
        }

        Snake_RenderGame(&game);
    }
}

static void App_HandlePause(uint8_t buttons)
{
    if(App_PauseComboPressed(buttons))
    {
        app_state = APP_GAME;
        game.state = GAME_RUNNING;
        last_step_ms = msTicks;
        button_latch = 0U;
        OLED_Clear();
        Snake_RenderInvalidate();
        Snake_RenderGame(&game);
    }
    else if(App_ConsumeButton(BUTTON_K4_STAR_MASK))
    {
        app_state = APP_MENU;
        button_latch = 0U;
        Menu_Render(&menu, high_score);
    }
}

static void App_HandleGameOver(void)
{
    if(App_ConsumeButton(BUTTON_K3_HASH_MASK))
    {
        App_StartGame();
    }
    else if(App_ConsumeButton(BUTTON_K4_STAR_MASK))
    {
        app_state = APP_MENU;
        button_latch = 0U;
        Menu_Render(&menu, high_score);
    }
}

void App_Init(void)
{
    TIM2_Init_1ms();
    DWT_Delay_Init();
    Buttons_Init();
    lcd_init();
    lcd_clear();
    I2C1_Init();
    OLED_Init();

    Menu_Init(&menu);
    App_RenderLcd(1U);
    Menu_Render(&menu, high_score);

    TempSensor_Init();
}

void App_Run(void)
{
    while(1)
    {
        uint8_t buttons = App_ReadButtons();
        if((uint32_t)(msTicks - last_lcd_ms) >= LCD_UPDATE_PERIOD_MS)
        {
            last_lcd_ms = msTicks;
            App_RenderLcd(0U);
        }
        button_latch |= buttons;

        if(app_state == APP_MENU)
        {
            App_HandleMenu();
        }
        else if(app_state == APP_GAME)
        {
            App_HandleGame(buttons);
        }
        else if(app_state == APP_PAUSE)
        {
            App_HandlePause(buttons);
        }
        else
        {
            App_HandleGameOver();
        }

        previous_buttons = buttons;
        delayMs(APP_LOOP_DELAY_MS);
    }
}
