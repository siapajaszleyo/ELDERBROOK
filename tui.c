#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "tui.h"
#include "ascii.h"
#include "input.h"

static int g_color = 0;

void tuiInit(void)
{
    /* Prefer UTF-8 so hearts/gems and box characters render in foot. */
    setlocale(LC_ALL, "");

    const char *term = getenv("TERM");
    g_color = (term != NULL && strcmp(term, "dumb") != 0);

    /* Stay in the main buffer so terminal scrollback remains available. */
    if (g_color)
        printf("\033[2J\033[H");
    else
        asciiClear();

    fflush(stdout);
}

void tuiShutdown(void)
{
    if (g_color)
        printf("\033[0m");
    fflush(stdout);
}

void tuiClear(void)
{
    if (g_color)
        printf("\033[2J\033[H");
    else
        asciiClear();
    fflush(stdout);
}

void tuiStatusStrip(const char *name, int level, int hp, int maxHp,
                    int mp, int maxMp, int gold, int cleared, int total)
{
    printf("\n");
    asciiRule(72);
    printf("  %s  Lv%d  |  HP %d/%d  MP %d/%d  |  %d gold  |  %d/%d cleared\n",
           name ? name : "Hero", level, hp, maxHp, mp, maxMp, gold,
           cleared, total);
    asciiRule(72);
}

void tuiPressEnter(void)
{
    printf("\n  Press Enter to continue... ");
    fflush(stdout);
    char line[8];
    readOptional(line, sizeof line);
}

void tuiSection(const char *title)
{
    asciiBanner(title);
}

void tuiColor(const char *ansiCode)
{
    if (g_color && ansiCode)
        printf("%s", ansiCode);
}

void tuiReset(void)
{
    if (g_color)
        printf("\033[0m");
}

int tuiHasColor(void)
{
    return g_color;
}
