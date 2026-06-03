#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "snake_types.h"

void Snake_Init(SnakeGame *game, Difficulty difficulty, uint16_t high_score);
void Snake_SetDirection(SnakeGame *game, Direction direction);
void Snake_Update(SnakeGame *game);
uint16_t Snake_GetStepPeriodMs(Difficulty difficulty);
const char *Snake_GetDifficultyName(Difficulty difficulty);

#endif
