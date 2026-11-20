#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dice.h"
#include "input.h"
#include "game.h"
#include "adventure.h"
#include "class.h"
#include "ascii.h"

#define DICE_COUNT      3
#define ROUNDS_TO_WIN   3

static int rollDie(void)
{
    return rand() % 6 + 1;
}

static int total(const int dice[DICE_COUNT])
{
    int sum = 0;

    for (int i = 0; i < DICE_COUNT; i++)
        sum += dice[i];

    return sum;
}

static void showDice(const char *who, const int dice[DICE_COUNT])
{
    printf("%-22s", who);
    for (int i = 0; i < DICE_COUNT; i++)
        printf("[%d] ", dice[i]);
    printf("= %d\n", total(dice));
}


int diceRound(const char *opponent, int dungeonLevel)
{
    int mine[DICE_COUNT];
    int theirs[DICE_COUNT];
    char who[64];

    for (int i = 0; i < DICE_COUNT; i++)
    {
        mine[i] = rollDie();
        theirs[i] = rollDie();
    }

    printf("\n");
    showDice("Your dice:", mine);

    printf("Re-roll which dice? (e.g. 1 3, or 'all'; Enter keeps; q gives up): ");

    char line[64];
    readOptional(line, sizeof line);

    if (tolower((unsigned char)line[0]) == 'q')
        return DICE_QUIT;

    int rerolled = 0;

    if (strncmp(line, "all", 3) == 0 || strncmp(line, "ALL", 3) == 0)
    {
        for (int i = 0; i < DICE_COUNT; i++)
            mine[i] = rollDie();
        rerolled = 1;
    }
    else
    {
        for (int i = 0; line[i] != '\0'; i++)
        {
            int index = line[i] - '1';

            if (index >= 0 && index < DICE_COUNT)
            {
                mine[index] = rollDie();
                rerolled = 1;
            }
        }
    }

    if (rerolled)
        showDice("After re-rolling:", mine);

    /* The opponent re-rolls its low dice once. Early on it is careless
       and never re-rolls; later it gets sharper. */
    int threshold = dungeonLevel <= 3 ? 0 : 3;
    int changed = 0;

    for (int i = 0; i < DICE_COUNT; i++)
    {
        if (theirs[i] <= threshold)
        {
            theirs[i] = rollDie();
            changed++;
        }
    }

    if (changed > 0)
        printf("%s re-rolls %d di%s.\n", opponent, changed, changed == 1 ? "e" : "ce");

    /* Gambler class + King's Coin: small chance to re-roll opponent's best die */
    int gambleBonus = classGamblingBonus(adv.playerClass) + relicGamblingBonus();
    if (gambleBonus > 0 && rand() % 100 < gambleBonus)
    {
        /* Find opponent's highest die and re-roll it */
        int bestIdx = 0;
        for (int i = 1; i < DICE_COUNT; i++)
            if (theirs[i] > theirs[bestIdx]) bestIdx = i;
        theirs[bestIdx] = rollDie();
        printf("[Fortune smiles] %s re-rolls their highest die!\n", opponent);
    }

    snprintf(who, sizeof who, "%s:", opponent);
    showDice(who, theirs);

    int mineTotal = total(mine);
    int theirTotal = total(theirs);

    if (mineTotal > theirTotal)
        return DICE_WIN;
    if (mineTotal < theirTotal)
        return DICE_LOSE;
    return DICE_TIE;
}


int playDice(int dungeonLevel, const char *bossName)
{
#ifdef RPG_TUI
    asciiDiceHeader();
#endif
    int myWins = 0;
    int theirWins = 0;

    printf("\n=================================\n");
    printf("        DICE DUEL\n");
    printf("=================================\n");
    printf("Each round: both roll three dice, you may re-roll some once,\n");
    printf("and the higher total wins the round. Ties are replayed.\n");
    printf("First to %d round wins takes the duel.\n", ROUNDS_TO_WIN);

    while (myWins < ROUNDS_TO_WIN && theirWins < ROUNDS_TO_WIN)
    {
        printf("\n--- Score: You %d - %d %s ---", myWins, theirWins, bossName);

        int result = diceRound(bossName, dungeonLevel);

        if (result == DICE_QUIT)
        {
            printf("You push back from the table.\n");
            return MINI_QUIT;
        }

        if (result == DICE_WIN)
        {
            myWins++;
            printf("You win the round!\n");
        }
        else if (result == DICE_LOSE)
        {
            theirWins++;
            printf("%s wins the round.\n", bossName);
        }
        else
        {
            printf("A tie! Roll again.\n");
        }
    }

    printf("\nFinal score: You %d - %d %s\n", myWins, theirWins, bossName);

    return myWins >= ROUNDS_TO_WIN ? MINI_WIN : MINI_LOSE;
}
