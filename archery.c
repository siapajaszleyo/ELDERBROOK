#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archery.h"
#include "input.h"
#include "game.h"
#include "ascii.h"

#define COLUMNS 5
#define ROWS    3
#define SHOTS   5

static void drawTarget(void)
{
    printf("\n            columns\n");
    printf("         1   2   3   4   5\n");
    for (int r = 1; r <= ROWS; r++)
    {
        printf("  row %d  ", r);
        for (int c = 1; c <= COLUMNS; c++)
        {
            int distance = abs(c - 3) + abs(r - 2);
            const char *mark = distance == 0 ? "(10)"
                             : distance == 1 ? " 6  "
                             : distance == 2 ? " 3  "
                             : distance == 3 ? " 1  "
                             : " .  ";
            printf("%s", mark);
        }
        printf("\n");
    }
}

static int scoreFor(int column, int row)
{
    if (column < 1 || column > COLUMNS || row < 1 || row > ROWS)
        return 0;

    switch (abs(column - 3) + abs(row - 2))
    {
        case 0:  return 10;
        case 1:  return 6;
        case 2:  return 3;
        case 3:  return 1;
        default: return 0;
    }
}

int playArchery(int level)
{
#ifdef RPG_TUI
    asciiArcheryHeader();
#endif
    int difficulty = level <= 5 ? 1 : (level <= 10 ? 2 : 3);
    int total = 0;

    printf("\n=================================\n");
    printf("          ARCHERY\n");
    printf("=================================\n");
    printf("%d arrows. The bullseye (10) is column 3, row 2.\n", SHOTS);
    printf("The wind pushes your arrow sideways: aim AGAINST it!\n");
    printf("Type your aim as column then row, e.g.  3 2\n");

    drawTarget();

    for (int shot = 1; shot <= SHOTS; shot++)
    {
        int wind = rand() % 5 - 2;                  /* -2 (west) .. +2 (east) */

        printf("\nArrow %d of %d.  Wind: ", shot, SHOTS);
        if (wind == 0)
            printf("calm\n");
        else
            printf("%d to the %s\n", abs(wind), wind > 0 ? "east (pushes right)" : "west (pushes left)");

        int column = 0, row = 0;

        while (1)
        {
            char line[32];

            printf("Aim (column 1-5, row 1-3): ");
            readLine(line, sizeof line);

            if (sscanf(line, "%d %d", &column, &row) == 2 &&
                column >= 1 && column <= COLUMNS && row >= 1 && row <= ROWS)
                break;

            printf("Please type two numbers, like  3 2\n");
        }

        /* Gusts: sometimes the wind is a little stronger or weaker */
        int actual = wind;

        if (rand() % 100 < 20 + difficulty * 5)
            actual += (rand() % 2) ? 1 : -1;

        int landColumn = column + actual;
        int landRow = row;

        if (difficulty >= 3 && rand() % 100 < 20)
            landRow += (rand() % 2) ? 1 : -1;         /* a trembling hand */

        int points = scoreFor(landColumn, landRow);

        printf("Your arrow lands at column %d, row %d: ", landColumn, landRow);

        if (points == 10)
            printf("BULLSEYE! +10\n");
        else if (points > 0)
            printf("+%d\n", points);
        else
            printf("a miss.\n");

        total += points;
    }

    printf("\nTotal score: %d / %d\n", total, SHOTS * 10);

    return total;
}

int archeryDuel(int level, const char *bossName)
{
    int needed = 20 + level;

    printf("\n%s: \"Score at least %d, or leave.\"\n", bossName, needed);

    int score = playArchery(level);

    return score >= needed ? MINI_WIN : MINI_LOSE;
}
