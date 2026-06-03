#include "../src/hfiles/snake_game.h"

static uint16_t rng_state = 0xACE1U;

static uint8_t Snake_IsOpposite(Direction a, Direction b)
{
    return ((a == DIR_UP) && (b == DIR_DOWN)) ||
           ((a == DIR_DOWN) && (b == DIR_UP)) ||
           ((a == DIR_LEFT) && (b == DIR_RIGHT)) ||
           ((a == DIR_RIGHT) && (b == DIR_LEFT));
}

static uint8_t Snake_Occupies(const SnakeGame *game, uint8_t x, uint8_t y)
{
    uint8_t i;

    for(i = 0U; i < game->length; i++)
    {
        if((game->body[i].x == x) && (game->body[i].y == y))
        {
            return 1U;
        }
    }

    return 0U;
}

static uint8_t Snake_IsCorner(uint8_t x, uint8_t y)
{
    return (uint8_t)(((x == 0U) || (x == (SNAKE_GRID_WIDTH - 1U))) &&
                     ((y == 0U) || (y == (SNAKE_GRID_HEIGHT - 1U))));
}

static uint16_t Snake_Rand(void)
{
    rng_state = (uint16_t)((rng_state * 1103515245U + 12345U) >> 1);
    return rng_state;
}

static void Snake_GenerateFood(SnakeGame *game)
{
    uint16_t attempts;
    SnakePoint candidate;

    for(attempts = 0U; attempts < 512U; attempts++)
    {
        candidate.x = (uint8_t)(Snake_Rand() % SNAKE_GRID_WIDTH);
        candidate.y = (uint8_t)(Snake_Rand() % SNAKE_GRID_HEIGHT);

        if(!Snake_IsCorner(candidate.x, candidate.y) &&
           !Snake_Occupies(game, candidate.x, candidate.y))
        {
            game->food = candidate;
            return;
        }
    }

    game->food.x = SNAKE_GRID_WIDTH / 2U;
    game->food.y = SNAKE_GRID_HEIGHT / 2U;
}

void Snake_Init(SnakeGame *game, Difficulty difficulty, uint16_t high_score)
{
    uint8_t start_x = SNAKE_GRID_WIDTH / 2U;
    uint8_t start_y = SNAKE_GRID_HEIGHT / 2U;

    game->state = GAME_RUNNING;
    game->difficulty = difficulty;
    game->direction = DIR_RIGHT;
    game->pending_direction = DIR_RIGHT;
    game->length = 3U;
    game->score = 0U;
    game->high_score = high_score;

    game->body[0].x = start_x;
    game->body[0].y = start_y;
    game->body[1].x = (uint8_t)(start_x - 1U);
    game->body[1].y = start_y;
    game->body[2].x = (uint8_t)(start_x - 2U);
    game->body[2].y = start_y;

    rng_state ^= (uint16_t)((difficulty + 1U) * 173U);
    Snake_GenerateFood(game);
}

void Snake_SetDirection(SnakeGame *game, Direction direction)
{
    if(direction == DIR_NONE)
    {
        return;
    }

    if(!Snake_IsOpposite(game->direction, direction))
    {
        game->pending_direction = direction;
    }
}

void Snake_Update(SnakeGame *game)
{
    SnakePoint next;
    uint8_t i;
    uint8_t ate_food;

    if(game->state != GAME_RUNNING)
    {
        return;
    }

    game->direction = game->pending_direction;
    next = game->body[0];

    if(game->direction == DIR_UP)
    {
        if(next.y == 0U)
        {
            game->state = GAME_OVER;
            return;
        }
        next.y--;
    }
    else if(game->direction == DIR_DOWN)
    {
        next.y++;
    }
    else if(game->direction == DIR_LEFT)
    {
        if(next.x == 0U)
        {
            game->state = GAME_OVER;
            return;
        }
        next.x--;
    }
    else if(game->direction == DIR_RIGHT)
    {
        next.x++;
    }

    if((next.x >= SNAKE_GRID_WIDTH) || (next.y >= SNAKE_GRID_HEIGHT))
    {
        game->state = GAME_OVER;
        return;
    }

    ate_food = (uint8_t)((next.x == game->food.x) && (next.y == game->food.y));

    for(i = 0U; i < (uint8_t)(game->length - (ate_food ? 0U : 1U)); i++)
    {
        if((game->body[i].x == next.x) && (game->body[i].y == next.y))
        {
            game->state = GAME_OVER;
            return;
        }
    }

    if(ate_food && (game->length < SNAKE_MAX_LENGTH))
    {
        game->length++;
        game->score = (uint16_t)(game->score + 10U);
        if(game->score > game->high_score)
        {
            game->high_score = game->score;
        }
    }
    else if(ate_food)
    {
        game->score = (uint16_t)(game->score + 10U);
        if(game->score > game->high_score)
        {
            game->high_score = game->score;
        }
        game->state = GAME_OVER;
        return;
    }

    for(i = game->length - 1U; i > 0U; i--)
    {
        game->body[i] = game->body[i - 1U];
    }

    game->body[0] = next;

    if(ate_food && (game->length < SNAKE_MAX_LENGTH))
    {
        Snake_GenerateFood(game);
    }
}

uint16_t Snake_GetStepPeriodMs(Difficulty difficulty)
{
    if(difficulty == DIFFICULTY_EASY)
    {
        return 1000U;
    }
    if(difficulty == DIFFICULTY_NORMAL)
    {
        return 160U;
    }

    return 100U;
}

const char *Snake_GetDifficultyName(Difficulty difficulty)
{
    if(difficulty == DIFFICULTY_EASY)
    {
        return "Easy";
    }
    if(difficulty == DIFFICULTY_NORMAL)
    {
        return "Normal";
    }

    return "Hard";
}
