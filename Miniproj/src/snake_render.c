#include "../src/hfiles/snake_render.h"
#include "../src/hfiles/OLED.h"
#include "../src/hfiles/snake_game.h"
#include "../src/hfiles/utils.h"

#define OLED_CHAR_WIDTH 6U

static char line_buffer[SNAKE_GRID_WIDTH + 3U];
static char cell_shadow[SNAKE_GRID_HEIGHT][SNAKE_GRID_WIDTH];
static uint8_t render_valid = 0U;

static void Render_Line(uint8_t page, const char *text)
{
    OLED_SetCursor(page, 0U);
    OLED_PrintString(text);
}

static char Snake_CellAt(const SnakeGame *game, uint8_t x, uint8_t y)
{
    uint8_t i;

    if((game->food.x == x) && (game->food.y == y))
    {
        return '*';
    }

    for(i = 0U; i < game->length; i++)
    {
        if((game->body[i].x == x) && (game->body[i].y == y))
        {
            return (i == 0U) ? '@' : 'o';
        }
    }

    return ' ';
}

static void Render_Cell(uint8_t x, uint8_t y, char value)
{
    OLED_SetCursor((uint8_t)(y + 2U), (uint8_t)((x + 1U) * OLED_CHAR_WIDTH));
    OLED_PrintCharASCII(value);
}

void Snake_RenderInvalidate(void)
{
    render_valid = 0U;
}

static void Render_GameFull(const SnakeGame *game)
{
    uint8_t x;
    uint8_t y;

    Render_Line(0U, "Snake Game           ");

    for(x = 0U; x < (uint8_t)(SNAKE_GRID_WIDTH + 2U); x++)
    {
        line_buffer[x] = '#';
    }
    line_buffer[SNAKE_GRID_WIDTH + 2U] = '\0';
    Render_Line(1U, line_buffer);

    for(y = 0U; y < SNAKE_GRID_HEIGHT; y++)
    {
        line_buffer[0] = '#';
        for(x = 0U; x < SNAKE_GRID_WIDTH; x++)
        {
            line_buffer[x + 1U] = Snake_CellAt(game, x, y);
            cell_shadow[y][x] = line_buffer[x + 1U];
        }
        line_buffer[SNAKE_GRID_WIDTH + 1U] = '#';
        line_buffer[SNAKE_GRID_WIDTH + 2U] = '\0';
        Render_Line((uint8_t)(y + 2U), line_buffer);
    }

    for(x = 0U; x < (uint8_t)(SNAKE_GRID_WIDTH + 2U); x++)
    {
        line_buffer[x] = '#';
    }
    line_buffer[SNAKE_GRID_WIDTH + 2U] = '\0';
    Render_Line(7U, line_buffer);

    render_valid = 1U;
}

void Snake_RenderGame(const SnakeGame *game)
{
    uint8_t x;
    uint8_t y;
    char current;

    if(render_valid == 0U)
    {
        Render_GameFull(game);
        return;
    }

    for(y = 0U; y < SNAKE_GRID_HEIGHT; y++)
    {
        for(x = 0U; x < SNAKE_GRID_WIDTH; x++)
        {
            current = Snake_CellAt(game, x, y);
            if(cell_shadow[y][x] != current)
            {
                Render_Cell(x, y, current);
                cell_shadow[y][x] = current;
            }
        }
    }
}

void Snake_RenderPause(const SnakeGame *game)
{
    Snake_RenderGame(game);
    OLED_SetCursor(3U, 42U);
    OLED_PrintString("PAUSE");
    OLED_SetCursor(4U, 18U);
    OLED_PrintString("K3+K4 resume");
}

void Snake_RenderGameOver(const SnakeGame *game)
{
    char score[12];
    char high[12];

    intToStr((int)game->score, score);
    intToStr((int)game->high_score, high);

    OLED_Clear();
    Render_Line(0U, "==== GAME OVER ====");
    Render_Line(2U, "Score:");
    OLED_SetCursor(2U, 42U);
    OLED_PrintString(score);
    Render_Line(3U, "High :");
    OLED_SetCursor(3U, 42U);
    OLED_PrintString(high);
    Render_Line(5U, "K3 # restart");
    Render_Line(6U, "K4 * menu");
}
