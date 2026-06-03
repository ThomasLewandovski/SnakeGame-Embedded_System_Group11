#ifndef SNAKE_TYPES_H
#define SNAKE_TYPES_H

#include <stdint.h>

#define SNAKE_GRID_WIDTH       19U
#define SNAKE_GRID_HEIGHT       5U
#define SNAKE_MAX_LENGTH      (SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT)

typedef enum
{
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef enum
{
    DIFFICULTY_EASY = 0,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD,
    DIFFICULTY_COUNT
} Difficulty;

typedef enum
{
    GAME_READY = 0,
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

typedef struct
{
    uint8_t x;
    uint8_t y;
} SnakePoint;

typedef struct
{
    GameState state;
    Difficulty difficulty;
    Direction direction;
    Direction pending_direction;
    SnakePoint body[SNAKE_MAX_LENGTH];
    uint8_t length;
    SnakePoint food;
    uint16_t score;
    uint16_t high_score;
} SnakeGame;

#endif
