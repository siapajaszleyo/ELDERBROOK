#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "reflex.h"
#include "input.h"
#include "util.h"
#include "game.h"
#include "ascii.h"

#define ROUNDS 4

int playReflex(int level, const char *bossName)
{
#ifdef RPG_TUI
    asciiReflexHeader();
#endif
    static const char keys[4] = { 'W', 'A', 'S', 'D' };
    static const char *words[4] = { "UP", "LEFT", "DOWN", "RIGHT" };

    int difficulty = level <= 5 ? 1 : (level <= 10 ? 2 : 3);
    int passed = 0;

    printf("\n=================================\n");
    printf("      REFLEX CHALLENGE\n");
    printf("=================================\n");
    printf("%s shouts a string of commands. Type the matching letters\n", bossName);
    printf("(W=UP  A=LEFT  S=DOWN  D=RIGHT) before time runs out!\n");
    printf("%d rounds. Pass 3 to win.\n", ROUNDS);

    for (int round = 1; round <= ROUNDS; round++)
    {
        int length = 3 + difficulty + round / 2;
        int seconds = 7 - difficulty + length / 4;      /* shrinks with level */
        char answer[16];

        for (int i = 0; i < length; i++)
            answer[i] = keys[rand() % 4];
        answer[length] = '\0';

        printf("\nRound %d: press Enter to start... ", round);
        char ready[16];
        readOptional(ready, sizeof ready);

        printf("\n   ");
        for (int i = 0; i < length; i++)
            for (int k = 0; k < 4; k++)
                if (keys[k] == answer[i])
                    printf("%s ", words[k]);
        printf("\n   (you have %d seconds)\n", seconds);
        printf("Type the letters: ");

        time_t start = time(NULL);
        char line[64];
        char typed[64];
        int n = 0;

        readLine(line, sizeof line);

        double elapsed = difftime(time(NULL), start);

        for (int i = 0; line[i] != '\0' && n < 60; i++)
            if (!isspace((unsigned char)line[i]))
                typed[n++] = (char)toupper((unsigned char)line[i]);
        typed[n] = '\0';

        if (!fastMode() && elapsed > seconds)
            printf("Too slow! (%.0f seconds)\n", elapsed);
        else if (strcmp(typed, answer) != 0)
            printf("Wrong sequence! It was %s.\n", answer);
        else
        {
            passed++;
            printf("Lightning fast!\n");
        }
    }

    printf("\nYou passed %d of %d rounds.\n", passed, ROUNDS);

    return passed >= 3 ? MINI_WIN : MINI_LOSE;
}
