#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "snake.h"
#include "input.h"
#include "game.h"
#include "ascii.h"

#define GRID            10      /* the board is GRID x GRID squares     */
#define START_LENGTH    2       /* metres at the start                  */
#define GOAL_LENGTH     15      /* metres needed to win                 */
#define CUT_AT_LENGTH   8       /* the miniboss cuts you down here      */
#define CUT_TO_LENGTH   1       /* ...to this many metres               */

typedef struct
{
    int row;
    int col;
} Cell;

/* body[0] is the head. Only the first `length` entries are used. */
static Cell body[GRID * GRID];
static int  length;
static Cell fruit;


static int isBody(int row, int col)
{
    for (int i = 0; i < length; i++)
        if (body[i].row == row && body[i].col == col)
            return 1;
    return 0;
}

static void placeFruit(void)
{
    do
    {
        fruit.row = rand() % GRID;
        fruit.col = rand() % GRID;
    }
    while (isBody(fruit.row, fruit.col));
}

static void drawBoard(const char *bossName)
{
    printf("\n  %s's SNAKE ARENA        Length: %dm / %dm\n",
           bossName, length, GOAL_LENGTH);
    printf("  +");
    for (int c = 0; c < GRID; c++)
        printf("--");
    printf("-+\n");

    /* Row GRID-1 is the top of the screen, so 'w' (up) means row + 1 */
    for (int r = GRID - 1; r >= 0; r--)
    {
        printf("  | ");
        for (int c = 0; c < GRID; c++)
        {
            char symbol = '.';

            if (fruit.row == r && fruit.col == c)
                symbol = '*';
            if (isBody(r, c))
                symbol = 'o';
            if (body[0].row == r && body[0].col == c)
                symbol = '@';

            printf("%c ", symbol);
        }
        printf("|\n");
    }

    printf("  +");
    for (int c = 0; c < GRID; c++)
        printf("--");
    printf("-+\n");
}

/* Try to move the head by (dRow, dCol).
   Returns 0 = moved, 1 = crashed, 2 = ignored (can't reverse). */
static int step(int dRow, int dCol, int *ate)
{
    Cell next = { body[0].row + dRow, body[0].col + dCol };

    *ate = 0;

    if (next.row < 0 || next.row >= GRID || next.col < 0 || next.col >= GRID)
        return 1;                               /* hit a wall */

    if (length > 1 && next.row == body[1].row && next.col == body[1].col)
        return 2;                               /* tried to turn back */

    int eating = (next.row == fruit.row && next.col == fruit.col);

    /* The tail moves out of the way unless we are growing */
    int check = eating ? length : length - 1;

    for (int i = 0; i < check; i++)
        if (body[i].row == next.row && body[i].col == next.col)
            return 1;                           /* bit yourself */

    int newLength = eating ? length + 1 : length;

    for (int i = newLength - 1; i > 0; i--)
        body[i] = body[i - 1];

    body[0] = next;
    length = newLength;
    *ate = eating;

    return 0;
}


int playSnake(int dungeonLevel, const char *bossName)
{
#ifdef RPG_TUI
    asciiSnakeHeader();
#endif
    (void)dungeonLevel;     /* reserved: could make the arena harder later */

    int cutDone = 0;

    length = START_LENGTH;
    body[0].row = GRID / 2;  body[0].col = GRID / 2;
    body[1].row = GRID / 2;  body[1].col = GRID / 2 - 1;
    placeFruit();

    printf("\n=================================\n");
    printf("        SNAKE DUEL\n");
    printf("=================================\n");
    printf("You are the snake (@ head, o body). Eat fruit (*) to grow.\n");
    printf("Start at %dm and reach %dm to win. Each fruit adds 1m.\n",
           START_LENGTH, GOAL_LENGTH);
    printf("Careful: %s will cut you down to %dm when you reach %dm!\n",
           bossName, CUT_TO_LENGTH, CUT_AT_LENGTH);
    printf("Walls and your own body are deadly.\n\n");
    printf("Type moves with w (up) a (left) s (down) d (right).\n");
    printf("You can type several at once, e.g.  ddws   Type q to give up.\n");

    while (1)
    {
        drawBoard(bossName);
        printf("\nMoves: ");

        char line[128];
        readLine(line, sizeof line);

        for (int i = 0; line[i] != '\0'; i++)
        {
            int dRow = 0, dCol = 0;
            char key = (char)tolower((unsigned char)line[i]);

            if (key == 'q')
            {
                printf("You slither away from the arena.\n");
                return MINI_QUIT;
            }
            else if (key == 'w') dRow = 1;
            else if (key == 's') dRow = -1;
            else if (key == 'a') dCol = -1;
            else if (key == 'd') dCol = 1;
            else if (isspace((unsigned char)key)) continue;
            else
            {
                printf("(ignored '%c': use w a s d)\n", line[i]);
                continue;
            }

            int ate;
            int result = step(dRow, dCol, &ate);

            if (result == 1)
            {
                drawBoard(bossName);
                printf("\nYou crashed! %s laughs.\n", bossName);
                return MINI_LOSE;
            }

            if (result == 2)
            {
                printf("(You can't turn straight back on yourself.)\n");
                continue;
            }

            if (ate)
            {
                placeFruit();
                printf("Yum! You are now %dm long.\n", length);

                if (!cutDone && length >= CUT_AT_LENGTH)
                {
                    cutDone = 1;
                    length = CUT_TO_LENGTH;
                    printf("\n*** %s lunges and CUTS you down to %dm! ***\n",
                           bossName, CUT_TO_LENGTH);
                    printf("Keep going! You must still reach %dm.\n", GOAL_LENGTH);
                }

                if (length >= GOAL_LENGTH)
                {
                    drawBoard(bossName);
                    printf("\nYou reached %dm! %s yields.\n", GOAL_LENGTH, bossName);
                    return MINI_WIN;
                }
            }
        }
    }
}
