#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "input.h"
#include "util.h"
#include "game.h"
#include "ascii.h"

#define ROUNDS 3

int playMemory(int level, const char *bossName)
{
#ifdef RPG_TUI
    asciiMemoryHeader();
#endif
    static const char *names[4]   = { "RED", "GREEN", "BLUE", "YELLOW" };
    static const char  letters[4] = { 'R', 'G', 'B', 'Y' };

    int difficulty = level <= 5 ? 1 : (level <= 10 ? 2 : 3);
    int passed = 0;

    printf("\n=================================\n");
    printf("        MEMORY TRIAL\n");
    printf("=================================\n");
    printf("%s flashes a pattern of colours, then hides it.\n", bossName);
    printf("Type it back using the first letters: R G B Y.\n");
    printf("%d rounds, each longer. Pass 2 to win.\n", ROUNDS);

    for (int round = 1; round <= ROUNDS; round++)
    {
        int length = 2 + difficulty + round;       /* 4..6 up to 6..8 */
        char answer[16];

        for (int i = 0; i < length; i++)
            answer[i] = letters[rand() % 4];
        answer[length] = '\0';

        printf("\nRound %d of %d: memorise this!   (press Enter when ready) ", round, ROUNDS);

        char ready[16];
        readOptional(ready, sizeof ready);

        printf("\n   ");
        for (int i = 0; i < length; i++)
        {
            for (int c = 0; c < 4; c++)
                if (letters[c] == answer[i])
                    printf("%s ", names[c]);
        }
        printf("\n");

        pauseMs(1500 + length * 600);
        scrollAway();

        printf("Type the colours as letters (e.g. RGBY): ");

        char line[64];
        char typed[64];
        int n = 0;

        readLine(line, sizeof line);

        for (int i = 0; line[i] != '\0' && n < 60; i++)
            if (!isspace((unsigned char)line[i]))
                typed[n++] = (char)toupper((unsigned char)line[i]);
        typed[n] = '\0';

        if (strcmp(typed, answer) == 0)
        {
            passed++;
            printf("Perfect memory!\n");
        }
        else
            printf("Wrong! It was %s.\n", answer);
    }

    printf("\nYou remembered %d of %d patterns.\n", passed, ROUNDS);

    return passed >= 2 ? MINI_WIN : MINI_LOSE;
}
