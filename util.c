#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

int fastMode(void)
{
    return getenv("RPG_FAST") != NULL;
}

void pauseMs(int ms)
{
    if (fastMode())
        return;

    fflush(stdout);

    clock_t end = clock() + (clock_t)ms * CLOCKS_PER_SEC / 1000;

    while (clock() < end)
        ;
}

void scrollAway(void)
{
    for (int i = 0; i < 45; i++)
        printf("\n");

    fflush(stdout);
}
