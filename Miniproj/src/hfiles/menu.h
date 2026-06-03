#ifndef MENU_H
#define MENU_H

#include "snake_types.h"

typedef struct
{
    Difficulty difficulty;
} MenuState;

void Menu_Init(MenuState *menu);
void Menu_NextDifficulty(MenuState *menu);
void Menu_PreviousDifficulty(MenuState *menu);
void Menu_Render(const MenuState *menu, uint16_t high_score);

#endif
