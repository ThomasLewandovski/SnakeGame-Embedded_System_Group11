#include "../src/hfiles/menu.h"
#include "../src/hfiles/OLED.h"
#include "../src/hfiles/snake_game.h"
#include "../src/hfiles/utils.h"

void Menu_Init(MenuState *menu)
{
    menu->difficulty = DIFFICULTY_NORMAL;
}

void Menu_NextDifficulty(MenuState *menu)
{
    if(menu->difficulty == DIFFICULTY_HARD)
    {
        menu->difficulty = DIFFICULTY_EASY;
    }
    else
    {
        menu->difficulty = (Difficulty)(menu->difficulty + 1);
    }
}

void Menu_PreviousDifficulty(MenuState *menu)
{
    if(menu->difficulty == DIFFICULTY_EASY)
    {
        menu->difficulty = DIFFICULTY_HARD;
    }
    else
    {
        menu->difficulty = (Difficulty)(menu->difficulty - 1);
    }
}

void Menu_Render(const MenuState *menu, uint16_t high_score)
{
    char high[12];

    intToStr((int)high_score, high);

    OLED_Clear();
    OLED_SetCursor(0U, 0U);
    OLED_PrintString("Snake Mini Project");
    OLED_SetCursor(2U, 0U);
    OLED_PrintString("Difficulty:");
    OLED_SetCursor(2U, 72U);
    OLED_PrintString(Snake_GetDifficultyName(menu->difficulty));
    OLED_SetCursor(4U, 0U);
    OLED_PrintString("High Score:");
    OLED_SetCursor(4U, 72U);
    OLED_PrintString(high);
    OLED_SetCursor(6U, 0U);
    OLED_PrintString("K1/K2 choose");
    OLED_SetCursor(7U, 0U);
    OLED_PrintString("K3 # start");
}
