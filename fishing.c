#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fishing.h"
#include "adventure.h"
#include "quest.h"
#include "input.h"
#include "ascii.h"

#define TENSION_LIMIT 10

static const char *junkNames[3]    = { "an old boot", "a rusty tin can", "a tangle of weeds" };
static const char *commonNames[3]  = { "a silver minnow", "a plump river trout", "a sleepy carp" };
static const char *rareNames[3]    = { "a Moonlit Trout", "a Crimson Pike", "a Ghost Eel" };
static const char *specialNames[2] = { "a glowing pearl", "a sealed bottle with a note inside" };

/* How hard each kind of catch pulls back */
static int strengthFor(int rarity)
{
    static const int strength[6] = { 0, 1, 2, 1, 2, 4 };

    return strength[rarity];
}

static int pickRarity(void)
{
    /* A better rod finds fewer boots and more treasure */
    int junk     = 25 - 4 * adv.rodLevel; if (junk < 5) junk = 5;
    int common   = 40;
    int rare     = 20 + 2 * adv.rodLevel;
    int treasure = 8 + adv.rodLevel;
    int special  = 5 + adv.rodLevel;
    int legend   = 2 + (adv.rodLevel >= 3 ? 1 : 0);

    int total = junk + common + rare + treasure + special + legend;
    int roll = rand() % total;

    if ((roll -= junk) < 0)     return FISH_JUNK;
    if ((roll -= common) < 0)   return FISH_COMMON;
    if ((roll -= rare) < 0)     return FISH_RARE;
    if ((roll -= treasure) < 0) return FISH_TREASURE;
    if ((roll -= special) < 0)  return FISH_SPECIAL;
    return FISH_LEGEND;
}

static void bar(const char *label, int value, int max)
{
    printf("%s [", label);
    for (int i = 0; i < max; i++)
        printf(i < value ? "#" : ".");
    printf("] %d/%d\n", value, max);
}

/* The tug-of-war. Returns 1 if the catch is landed. */
static int reelIn(int rarity)
{
    int strength = strengthFor(rarity);

    if (strength == 0)
    {
        printf("It comes up with hardly a fight.\n");
        return 1;
    }

    int need = 6 + 2 * strength;
    int progress = 0;
    int tension = 2;

    printf("Something HEAVY is on the line! Reel it in without snapping the line.\n");

    for (int turn = 1; turn <= 16; turn++)
    {
        bar("  Tension ", tension, TENSION_LIMIT);
        bar("  Progress", progress > need ? need : progress, need);
        printf("1. Reel in   2. Ease off   3. Cut the line: ");

        int choice = readInt();

        if (choice == 3)
        {
            printf("You cut the line and let it go.\n");
            return 0;
        }

        if (choice == 2)
        {
            tension -= 3;
            if (tension < 0)
                tension = 0;
            progress += rand() % 2;
        }
        else
        {
            progress += 2 + rand() % 2;
            tension += 2 + rand() % 2;
        }

        /* The fish pulls back */
        tension += rand() % (1 + (strength + 1) / 2);

        if (tension >= TENSION_LIMIT)
        {
            printf("SNAP! The line breaks and it swims away.\n");
            return 0;
        }

        if (progress >= need)
            return 1;
    }

    printf("It slips off the hook and is gone.\n");
    return 0;
}

static void reward(int level, int rarity)
{
    int gold;

    adv.stats[ST_FISH]++;

    switch (rarity)
    {
    case FISH_JUNK:
        gold = 1 + rand() % 5;
        printf("You hauled up %s. You find %d gold stuck to it.\n", junkNames[rand() % 3], gold);
        player.gold += gold;
        adv.stats[ST_FISH]--;                   /* junk is not a fish */
        break;

    case FISH_COMMON:
        gold = 8 + level * 3;
        printf("You caught %s! You sell it for %d gold.\n", commonNames[rand() % 3], gold);
        player.gold += gold;
        break;

    case FISH_RARE:
        gold = 35 + level * 6;
        printf("A RARE catch: %s! It sells for %d gold.\n", rareNames[rand() % 3], gold);
        player.gold += gold;
        adv.stats[ST_RARE_FISH]++;
        questNotify(QN_FISH_RARE, 0);
        break;

    case FISH_TREASURE:
        gold = 50 + level * 8;
        printf("You reeled in a barnacled treasure chest! Inside: %d gold.\n", gold);
        player.gold += gold;
        grantMaterial(rand() % 10 == 0 ? MAT_RUNE : rand() % 5, 1 + rand() % 2);
        adv.stats[ST_TREASURES]++;
        adv.stats[ST_FISH]--;
        break;

    case FISH_SPECIAL:
        printf("Something special: %s!\n", specialNames[rand() % 2]);
        if (rand() % 2)
        {
            printf("It holds a healing elixir!\n");
            player.elixirs++;
        }
        else
            grantMaterial(MAT_RUNE, 1);
        adv.stats[ST_RARE_FISH]++;
        questNotify(QN_FISH_RARE, 0);
        break;

    case FISH_LEGEND:
        printf("\n*** IT IS THE GOLDEN KOI, a legend among fishermen! ***\n");
        printf("It sparkles with old magic. You gain 300 gold, two Ancient Runes\n");
        printf("and a surge of wisdom (+1 skill point).\n");
        player.gold += 300;
        grantMaterial(MAT_RUNE, 2);
        player.skillPoints++;
        adv.stats[ST_RARE_FISH]++;
        adv.stats[ST_LEGEND_FISH]++;
        questNotify(QN_FISH_RARE, 0);
        break;
    }
}

void fishingCatch(int level, int rarity)
{
    if (reelIn(rarity))
        reward(level, rarity);
}

void fishingSpot(int level)
{
#ifdef RPG_TUI
    asciiFishingHeader();
#endif
    int casts = 3 + (adv.rodLevel > 2 ? 2 : adv.rodLevel);

    printf("\n=================================\n");
    printf("          FISHING\n");
    printf("=================================\n");
    printf("You find a quiet bank. You can cast up to %d times here.\n", casts);

    for (int cast = 1; cast <= casts; cast++)
    {
        printf("\nCast %d of %d:  1. Cast your line   2. Pack up: ", cast, casts);

        if (readInt() != 1)
        {
            printf("You pack up your rod.\n");
            return;
        }

        printf("You wait... ");

        int rarity = pickRarity();

        if (rarity == FISH_JUNK)
            printf("a tug!\n");
        else if (rarity == FISH_LEGEND)
            printf("the water goes strangely still, and then... a HUGE tug!\n");
        else
            printf("a bite!\n");

        fishingCatch(level, rarity);
    }

    printf("\nThat's all the time you have here.\n");
}
