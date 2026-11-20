#include <stdio.h>
#include <stdlib.h>

#include "encounters.h"
#include "adventure.h"
#include "ui.h"
#include "monster.h"
#include "events.h"
#include "reflex.h"
#include "dice.h"
#include "input.h"
#include "achievements.h"

/* ---------------------------------------------------------
   Each function below is one kind of random encounter.
   --------------------------------------------------------- */

static void wanderingMerchant(void)
{
    printf("\nA wandering merchant waves from the roadside.\n");
    printf("\"Cheaper than the shops in town, friend, because I have no rent!\"\n");
    printf("You have %d gold.\n", player.gold);
    printf("1. Health Potion (35 gold)\n");
    printf("2. Mana Potion (20 gold)\n");
    printf("3. Fire Bomb (45 gold)\n");
    printf("4. Leave\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if (choice == 1 && player.gold >= 35)
    {
        player.gold -= 35;
        player.potions++;
        printf("You buy a health potion.\n");
    }
    else if (choice == 2 && player.gold >= 20)
    {
        player.gold -= 20;
        player.manaPotions++;
        printf("You buy a mana potion.\n");
    }
    else if (choice == 3 && player.gold >= 45)
    {
        player.gold -= 45;
        player.fireBombs++;
        printf("You buy a fire bomb.\n");
    }
    else if (choice >= 1 && choice <= 3)
        printf("You can't afford that.\n");
    else
        printf("You wave him goodbye.\n");
}

static void thief(int level)
{
    int stolen = player.gold / 6;

    printf("\nSomething brushes your belt. A THIEF has snatched your purse!\n");

    if (stolen < 5)
    {
        printf("He opens it, finds almost nothing, and tosses it back in disgust.\n");
        return;
    }

    printf("He takes %d gold and bolts down the path.\n", stolen);
    printf("1. Chase him down\n2. Let him go\nEnter the number of your choice: ");

    if (readInt() != 1)
    {
        player.gold -= stolen;
        printf("You let him go. (-%d gold)\n", stolen);
        return;
    }

    Enemy thiefEnemy;

    initializeEnemy(&thiefEnemy, "Cutpurse", level,
                    40 + level * 6, 8 + level, 3,
                    20 + level * 3, stolen + level * 5, SPECIAL_DOUBLE);

    int result = startCombat(&player, &thiefEnemy, 0);

    if (result == 1)
        printf("You take back what he stole, and a little more.\n");
}

static void treasure(int level)
{
    int gold = 25 + level * 8;

    printf("\nTucked under a loose stone you find a forgotten stash!\n");
    printf("+%d gold.\n", gold);
    player.gold += gold;
    grantMaterial(rand() % 10 == 0 ? MAT_RUNE : rand() % 5, 1);
    adv.stats[ST_TREASURES]++;
}

static void ambush(int level)
{
    Enemy enemy;

    printf("\nAMBUSH! Something leaps from cover!\n");
    spawnMonster(&enemy, level);

    if (startCombat(&player, &enemy, 1) == 1)
        printf("You survive the ambush.\n");
}

static void lostTraveller(int level)
{
    printf("\nA lost traveller sits by the road, clutching a torn map.\n");
    printf("1. Guide them to safety\n2. Walk on\nEnter the number of your choice: ");

    if (readInt() != 1)
    {
        printf("You walk on. They watch you go.\n");
        return;
    }

    int gold = 20 + level * 4;

    printf("You guide them to the next crossroads. They press %d gold into your hand.\n", gold);
    player.gold += gold;

    if (rand() % 100 < 20)
    {
        printf("\"Take this too,\" they say, \"a little elixir from my pack.\"\n");
        player.elixirs++;
    }

    gainXP(&player, 20 + level * 3);
}

static void shrine(void)
{
    printf("\nA strange little shrine glows beside the path.\n");
    printf("1. Leave an offering (30 gold)\n");
    printf("2. Smash it and take what's inside\n");
    printf("3. Leave it alone\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    adv.stats[ST_SHRINES]++;

    if (choice == 1)
    {
        if (player.gold < 30)
        {
            printf("You can't afford an offering.\n");
            return;
        }

        player.gold -= 30;

        int roll = rand() % 100;

        if (roll < 40)
        {
            player.maxHealth += 5;
            player.health += 5;
            printf("The shrine glows warm: +5 maximum health, permanently!\n");
        }
        else if (roll < 70)
        {
            player.health = player.maxHealth;
            player.mana = player.maxMana;
            printf("The shrine heals you completely.\n");
        }
        else
            printf("The shrine accepts your gold in silence. Nothing happens.\n");
    }
    else if (choice == 2)
    {
        if (rand() % 100 < 50)
        {
            int gold = 60 + progress.dungeonsCleared * 5;

            printf("Gold coins spill out! +%d gold.\n", gold);
            player.gold += gold;
        }
        else
        {
            printf("A curse lashes out! You lose 10 health.\n");
            player.health -= 10;
            if (player.health < 1)
                player.health = 1;
            showHealthBar();
        }
    }
    else
        printf("You leave the shrine alone.\n");
}

static void trap(int level)
{
    printf("\nCLICK! You step on a hidden pressure plate. Darts are about to fly!\n");
    printf("Dodge them with quick reflexes!\n");

    if (playReflex(level, "The trap") == MINI_WIN)
        printf("You dodge every dart. Not a scratch!\n");
    else
    {
        int damage = 8 + level * 2;

        printf("The darts find you: -%d health.\n", damage);
        player.health -= damage;
        if (player.health < 1)
            player.health = 1;
        adv.stats[ST_TRAPS]++;
        showHealthBar();
    }
}

static void travellingGambler(int level)
{
    printf("\nA travelling gambler hails you, rattling dice in his palm.\n");
    printf("\"One round for twenty gold. Highest total wins.\"\n");

    if (player.gold < 20)
    {
        printf("You don't have the gold, so you wave and walk on.\n");
        return;
    }

    printf("1. Play (20 gold)\n2. Walk on\nEnter the number of your choice: ");

    if (readInt() != 1)
        return;

    int result = diceRound("The gambler", level);

    if (result == DICE_WIN)
    {
        printf("You win 20 gold!\n");
        player.gold += 20;
        adv.stats[ST_GAMBLE_WINS]++;
    }
    else if (result == DICE_TIE)
        printf("A tie. Keep your gold.\n");
    else
    {
        printf("You lose 20 gold.\n");
        player.gold -= 20;
    }
}

void randomEncounter(int level)
{
    printf("\n~ Along the road, something unexpected happens... ~\n");

    int roll = rand() % 100;

    if (roll < 15)      wanderingMerchant();
    else if (roll < 27) thief(level);
    else if (roll < 42) treasure(level);
    else if (roll < 54) ambush(level);
    else if (roll < 64) lostTraveller(level);
    else if (roll < 76) shrine();
    else if (roll < 86) trap(level);
    else if (roll < 94) chestEvent(level);          /* the mysterious chest */
    else                travellingGambler(level);

    achievementCheck();
}
