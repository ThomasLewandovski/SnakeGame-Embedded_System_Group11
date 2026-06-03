#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include "snake_types.h"

void Snake_RenderInvalidate(void);
void Snake_RenderGame(const SnakeGame *game);
void Snake_RenderPause(const SnakeGame *game);
void Snake_RenderGameOver(const SnakeGame *game);

#endif
