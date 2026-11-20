#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lockpick.h"
#include "input.h"
#include "game.h"
#include "ascii.h"

/* The lock has 3 or 4 pins numbered 1-6, none repeated. After each guess
   you are told how many pins are right and in the right place, and how
   many are right but in the wrong place. */
int playLockpick(int level, const char *lockName)
{
#ifdef RPG_TUI
    asciiLockpickHeader();
#endif
    int length = level <= 6 ? 3 : 4;
    int attempts = level <= 6 ? 6 : 8;
    int code[4];
    int used[7] = {0};

    for (int i = 0; i < length; i++)
    {
        int digit;

        do
        {
            digit = 1 + rand() % 6;
        }
        while (used[digit]);

        used[digit] = 1;
        code[i] = digit;
    }

    printf("\n=================================\n");
    printf("        LOCKPICKING\n");
    printf("=================================\n");
    printf("%s has %d pins, each numbered 1 to 6, none repeated.\n", lockName, length);
    printf("Guess the combination (e.g. %s). After each try you learn:\n", length == 3 ? "123" : "1234");
    printf("  PLACED = right pin in the right place\n");
    printf("  LOOSE  = right pin in the wrong place\n");
    printf("You have %d attempts. Type q to give up.\n", attempts);

    for (int attempt = 1; attempt <= attempts; attempt++)
    {
        int guess[4];
        char line[32];

        while (1)
        {
            printf("\nAttempt %d of %d: ", attempt, attempts);
            readLine(line, sizeof line);

            if (line[0] == 'q' || line[0] == 'Q')
            {
                printf("You put your picks away.\n");
                return MINI_QUIT;
            }

            int ok = (int)strlen(line) == length;

            for (int i = 0; ok && i < length; i++)
            {
                if (line[i] < '1' || line[i] > '6')
                    ok = 0;
                else
                    guess[i] = line[i] - '0';
            }

            if (ok)
                break;

            printf("Enter exactly %d digits from 1 to 6.\n", length);
        }

        int placed = 0;
        int loose = 0;

        for (int i = 0; i < length; i++)
        {
            if (guess[i] == code[i])
                placed++;
            else
                for (int j = 0; j < length; j++)
                    if (guess[i] == code[j])
                    {
                        loose++;
                        break;
                    }
        }

        if (placed == length)
        {
            printf("CLICK! The lock opens!\n");
            return MINI_WIN;
        }

        printf("PLACED: %d   LOOSE: %d\n", placed, loose);
    }

    printf("\nThe lock stays shut. (It was ");
    for (int i = 0; i < length; i++)
        printf("%d", code[i]);
    printf(".)\n");

    return MINI_LOSE;
}
