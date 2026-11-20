#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "events.h"
#include "adventure.h"
#include "ui.h"
#include "world.h"
#include "monster.h"
#include "chess.h"
#include "snake.h"
#include "dice.h"
#include "memory.h"
#include "reflex.h"
#include "lockpick.h"
#include "archery.h"
#include "blackjack.h"
#include "fishing.h"
#include "npc.h"
#include "quest.h"
#include "achievements.h"
#include "secrets.h"
#include "input.h"



void encounterMonster(int dungeonLevel)
{
    Enemy enemy;

    spawnMonster(&enemy, dungeonLevel);

    printf("\n");
    printf(getLocation(dungeonLevel)->monsterIntro, enemy.name);
    printf("\n");

    int result = startCombat(&player, &enemy, 1);

    if (result == 1)
        printf("You defeated the monster!\n");
    else if (result == 0)
        printf("You escaped!\n");
    else
        printf("You have been defeated.\n");
}

void findItem(void)
{
    int item = rand() % 3;

    if (item == 0)
    {
        printf("\nYou find a health potion!\n");
        player.potions++;
    }
    else if (item == 1)
    {
        printf("\nYou find a mana potion!\n");
        player.manaPotions++;
    }
    else
    {
        printf("\nYou found 50 gold doubloons!\n");
        player.gold += 50;
    }
}

void merchantEncounter(void)
{
    printf("\nYou encounter a merchant in the dungeon!\n");
    printf("You have %d gold.\n", player.gold);
    printf("1. Buy Health Potion (50 gold)\n");
    printf("2. Buy Mana Potion (30 gold)\n");

    if (player.swordUpgrades < MAX_SWORD_UPGRADES)
        printf("3. Buy Sword Upgrade (100 gold, %d left)\n",
               MAX_SWORD_UPGRADES - player.swordUpgrades);
    else
        printf("3. Sword Upgrade (sold out)\n");

    printf("4. Leave\n");
    printf("Enter the number of your choice: ");

    switch (readInt())
    {
    case 1:
        if (player.gold >= 50)
        {
            printf("You bought a health potion.\n");
            player.gold -= 50;
            player.potions++;
        }
        else
            printf("Not enough gold!\n");
        break;

    case 2:
        if (player.gold >= 30)
        {
            printf("You bought a mana potion.\n");
            player.gold -= 30;
            player.manaPotions++;
        }
        else
            printf("Not enough gold!\n");
        break;

    case 3:
        if (player.swordUpgrades >= MAX_SWORD_UPGRADES)
            printf("\"I can't make that blade any sharper,\" the merchant says.\n");
        else if (player.gold >= 100)
        {
            printf("You upgraded your sword! Attack power increased.\n");
            player.gold -= 100;
            grantOffense(&player, 10);
            player.swordUpgrades++;
        }
        else
            printf("Not enough gold!\n");
        break;

    case 4:
        printf("You leave the merchant.\n");
        break;

    default:
        printf("Invalid choice. The merchant leaves.\n");
    }
}

void foragingOption(void)
{
    printf("\nYou find a quiet place to forage for supplies.\n");
    printf("1. Look for herbs (restore 15 health)\n");
    printf("2. Search for mana plants (restore 10 mana)\n");
    printf("3. Move on\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if ((choice == 1 || choice == 2) && rand() % 100 < 60)
        grantMaterial(rand() % 2 ? MAT_HERB : MAT_WOOD, 1);

    switch (choice)
    {
    case 1:
        healPlayer(&player, 15);
        printf("You found some healing herbs. Health restored by 15.\n");
        showHealthBar();
        break;

    case 2:
        restoreMana(&player, 10);
        printf("You found some mana plants. Mana restored by 10.\n");
        showManaMeter();
        break;

    case 3:
        printf("You decide to move on without foraging.\n");
        break;

    default:
        printf("Invalid choice.\n");
    }
}

/* An optional gambling table: one round of dice for a bet. */
void diceTable(int dungeonLevel)
{
    printf("\nA shady gambler waves you over to a dice table.\n");
    printf("\"Care for a wager, traveller? Highest total wins.\"\n");
    printf("You have %d gold.\n", player.gold);
    printf("1. Bet 10 gold\n");
    printf("2. Bet 30 gold\n");
    printf("3. Bet 60 gold\n");
    printf("4. Walk away\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();
    int bet = 0;

    if (choice == 1) bet = 10;
    else if (choice == 2) bet = 30;
    else if (choice == 3) bet = 60;

    if (bet == 0)
    {
        printf("You walk away from the table.\n");
        return;
    }

    if (player.gold < bet)
    {
        printf("You can't afford that bet.\n");
        return;
    }

    int result = diceRound("The gambler", dungeonLevel);

    if (result == DICE_WIN)
    {
        printf("You win %d gold!\n", bet);
        player.gold += bet;
        adv.stats[ST_GAMBLE_WINS]++;
    }
    else if (result == DICE_LOSE)
    {
        printf("You lose %d gold.\n", bet);
        player.gold -= bet;
    }
    else if (result == DICE_TIE)
    {
        printf("A tie. Your bet is returned.\n");
    }
    else
    {
        printf("You fold, and the gambler pockets %d gold.\n", bet);
        player.gold -= bet;
    }
}




/* ---------------------------------------------------------
   MINIBOSSES. Every location ends with one, and each location has its
   own. You can accept the challenge (a minigame) or fight with steel.
   Losing a minigame costs some health, then you may try again.
   --------------------------------------------------------- */

static int playMinigame(int kind, int level, const char *name)
{
    switch (kind)
    {
    case MG_CHESS:
    {
        int botLevel = level <= 3 ? 1 : (level <= 7 ? 2 : 3);

        setChessOpponent(name);
        return playChess(botLevel);
    }
    case MG_SNAKE:     return playSnake(level, name);
    case MG_DICE:      return playDice(level, name);
    case MG_BLACKJACK: return playBlackjackDuel(name);
    case MG_MEMORY:    return playMemory(level, name);
    case MG_REFLEX:    return playReflex(level, name);
    case MG_ARCHERY:   return archeryDuel(level, name);
    case MG_LOCKPICK:  return playLockpick(level, name);
    }

    return MINI_LOSE;
}

static void minigameWon(int kind)
{
    adv.stats[ST_MINIBOSS_WINS]++;

    if (kind == MG_CHESS) adv.stats[ST_CHESS_WINS]++;
    if (kind == MG_SNAKE) adv.stats[ST_SNAKE_WINS]++;
    if (kind == MG_DICE)  adv.stats[ST_DICE_WINS]++;
    if (kind == MG_BLACKJACK) adv.stats[ST_BJ_TOURNEY]++;
    if (kind == MG_MEMORY)    adv.stats[ST_MEMORY_WINS]++;
    if (kind == MG_REFLEX)    adv.stats[ST_REFLEX_WINS]++;
    if (kind == MG_LOCKPICK)  adv.stats[ST_LOCKPICK_WINS]++;
}

void minibossEncounter(int dungeonLevel)
{
    static const char *games[MG_COUNT] =
    {
        "a game of chess", "a game of snake", "a duel of dice",
        "a hand of blackjack", "a trial of memory", "a test of reflexes",
        "an archery contest", "a lock-picking contest"
    };

    const Location *where = getLocation(dungeonLevel);
    const char *name = where->minibossName;
    int kind = where->minibossKind;

    printf("\n=================================\n");
    printf("   MINIBOSS: %s\n", name);
    printf("=================================\n");
    printf("%s\n", where->minibossIntro);
    printf("%s blocks the way and challenges you to %s!\n", name, games[kind]);

    while (1)
    {
        printf("\n1. Accept the challenge\n");
        printf("2. Fight with sword and spell\n");
        printf("Enter the number of your choice: ");

        int choice = readInt();

        if (choice == 2)
        {
            Enemy boss;

            spawnMiniboss(&boss, dungeonLevel, name);

            int result = startCombat(&player, &boss, 0);

            if (result == 1)
            {
                adv.stats[ST_MINIBOSS_WINS]++;
                printf("\n%s falls!\n", name);
            }

            return;     /* won, or the hero died (the caller checks) */
        }

        if (choice != 1)
        {
            printf("Invalid choice.\n");
            continue;
        }

        int result = playMinigame(kind, dungeonLevel, name);

        if (result == MINI_WIN)
        {
            int gold = 60 + dungeonLevel * 20;

            printf("\n%s bows and hands over %d gold.\n", name, gold);
            player.gold += gold;
            gainXP(&player, 40 + dungeonLevel * 10);
            minigameWon(kind);
            return;
        }

        if (result == 2)            /* chess draw */
            printf("\n\"A draw? Play me again, then.\"\n");
        else
        {
            /* Costs health, but never the last point of it */
            player.health -= 15;
            if (player.health < 1)
                player.health = 1;

            printf("\n%s strikes you as you leave the board. -15 health.\n", name);
            showHealthBar();
        }
    }
}


/* ---------------------------------------------------------
   STORY SCENES. Each location opens with a short scene and a choice:
   the careful option is a small safe reward, the risky one is a gamble.
   --------------------------------------------------------- */

void storyVignette(int dungeonLevel)
{
    const Location *where = getLocation(dungeonLevel);

    printf("\n--- %s ---\n", where->name);
    printf("%s\n", where->story);
    printf("1. %s\n", where->storyA);
    printf("2. %s\n", where->storyB);
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if (choice == 2)
    {
        if (rand() % 100 < 55)
        {
            int gold = 30 + dungeonLevel * 12;

            printf("The gamble pays off! You gain %d gold.\n", gold);
            player.gold += gold;
            grantMaterial(rand() % 10 == 0 ? MAT_RUNE : rand() % 4, 1);
        }
        else
        {
            int damage = 8 + dungeonLevel;

            printf("It backfires! You lose %d health.\n", damage);
            player.health -= damage;
            if (player.health < 1)
                player.health = 1;
            showHealthBar();
        }
    }
    else
    {
        int gold = 20 + dungeonLevel * 5;

        printf("Careful work pays off. You gain %d gold.\n", gold);
        player.gold += gold;

        if (rand() % 100 < 40)
            grantMaterial(rand() % 4, 1);
    }
}



/* ---------------------------------------------------------
   NEW EVENTS: chests, mining, archery, cards, fishing
   --------------------------------------------------------- */

static void springTrap(int level)
{
    int damage = 10 + level * 2;

    printf("CLICK! A trap springs: you lose %d health.\n", damage);
    player.health -= damage;
    if (player.health < 1)
        player.health = 1;
    adv.stats[ST_TRAPS]++;
    showHealthBar();
}

void chestEvent(int level)
{
    printf("\nYou find a locked chest half-buried in the dust.\n");
    printf("1. Pick the lock carefully\n");
    printf("2. Smash it open\n");
    printf("3. Leave it alone\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();
    int opened = 0;
    int smashed = 0;

    if (choice == 1)
    {
        opened = playLockpick(level, "The chest lock") == MINI_WIN;
        if (opened)
            adv.stats[ST_LOCKPICK_WINS]++;
    }
    else if (choice == 2)
    {
        smashed = 1;
        opened = rand() % 100 < 50;
        if (opened)
            printf("The lid splinters open!\n");
    }
    else
    {
        printf("You leave the chest alone.\n");
        return;
    }

    if (!opened)
    {
        springTrap(level);
        return;
    }

    int gold = (40 + level * 10) / (smashed ? 2 : 1);

    printf("Inside you find %d gold!\n", gold);
    player.gold += gold;
    grantMaterial(rand() % 8 == 0 ? MAT_RUNE : rand() % 5, 1 + rand() % 2);

    if (rand() % 100 < 15)
    {
        printf("And a small bundle of supplies: a Fire Bomb!\n");
        player.fireBombs++;
    }

    adv.stats[ST_CHESTS]++;
    adv.stats[ST_TREASURES]++;
    questNotify(QN_CHEST, 0);
}

void mineEvent(int level)
{
    printf("\nA glittering vein of ore runs through the rock.\n");
    printf("1. Chip at it carefully\n");
    printf("2. Swing hard (more ore, but the roof may crumble)\n");
    printf("3. Leave it\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if (choice == 1)
    {
        grantMaterial(MAT_IRON, 1 + rand() % 2);
        if (rand() % 100 < 30)
            grantMaterial(MAT_CRYSTAL, 1);
    }
    else if (choice == 2)
    {
        grantMaterial(MAT_IRON, 2 + rand() % 3);
        grantMaterial(MAT_CRYSTAL, 1 + rand() % 2);

        if (level >= 8 && rand() % 100 < 25)
            grantMaterial(MAT_SCALE, 1);

        if (rand() % 100 < 35)
        {
            int damage = 8 + level;

            printf("The ceiling collapses on you! -%d health.\n", damage);
            player.health -= damage;
            if (player.health < 1)
                player.health = 1;
            showHealthBar();
        }
    }
    else
        printf("You leave the vein alone.\n");
}

void archeryEvent(int level)
{
    printf("\nAn old archery range, with a bow and a quiver of arrows.\n");
    printf("1. Take a turn\n2. Walk on\nEnter the number of your choice: ");

    if (readInt() != 1)
        return;

    int score = playArchery(level);

    if (score > adv.stats[ST_ARCHERY_BEST])
        adv.stats[ST_ARCHERY_BEST] = score;

    if (score >= 40)
    {
        printf("Master marksman! You win %d gold and a Crystal.\n", 100 + level * 10);
        player.gold += 100 + level * 10;
        grantMaterial(MAT_CRYSTAL, 1);
    }
    else if (score >= 28)
    {
        printf("Fine shooting! You win %d gold.\n", 50 + level * 5);
        player.gold += 50 + level * 5;
    }
    else if (score >= 15)
    {
        printf("Not bad. You win %d gold.\n", 20 + level * 2);
        player.gold += 20 + level * 2;
    }
    else
        printf("Better luck next time.\n");
}

void blackjackTable(int level)
{
    (void)level;

    printf("\nA card table, a dealer, and a stack of chips.\n");
    printf("\"Blackjack pays three to two, friend.\"\n");

    while (1)
    {
        printf("\nYou have %d gold.\n", player.gold);
        printf("1. Bet 10\n2. Bet 30\n3. Bet 60\n4. Leave the table\n");
        printf("Enter the number of your choice: ");

        int choice = readInt();
        int bet = choice == 1 ? 10 : (choice == 2 ? 30 : (choice == 3 ? 60 : 0));

        if (bet == 0)
        {
            printf("You leave the table.\n");
            return;
        }

        if (player.gold < bet)
        {
            printf("You can't afford that bet.\n");
            continue;
        }

        int net;
        int result = blackjackHand(bet, &net);

        player.gold += net;

        if (result == BJ_WIN || result == BJ_BLACKJACK)
        {
            adv.stats[ST_BJ_WINS]++;
            adv.stats[ST_GAMBLE_WINS]++;

            if (bet >= 30)
                questNotify(QN_BJ_BIG_WIN, 0);
        }
    }
}

void fishingEvent(int level)
{
    printf("\nYou come across good fishing water.\n");
    printf("1. Fish here\n2. Walk on\nEnter the number of your choice: ");

    if (readInt() == 1)
        fishingSpot(level);
}

/* =========================================================
   PHASE 5b: DUNGEON VARIETY ROOMS
   Four new self-contained room types that break up the
   fight / explore / fight rhythm.
   ========================================================= */

/* EV_ELITE: one tougher-than-normal fight in place of trash mobs.
   We take a normal monster from the location pool and promote it to
   an "elite" with more health, a sharper edge, and better rewards.
   Elites block the way, so there is no running. */
void eliteEncounter(int dungeonLevel)
{
    Enemy elite;
    char base[50];

    spawnMonster(&elite, dungeonLevel);

    static const char *prefixes[] =
        { "Dread", "Elite", "Warlord", "Ancient", "Savage" };
    const char *prefix = prefixes[rand() % 5];

    snprintf(base, sizeof base, "%s", elite.name);
    /* Bounded precisions keep the titled name within elite.name[50]. */
    snprintf(elite.name, sizeof elite.name, "%.8s %.40s", prefix, base);

    /* Promote the stat line. */
    elite.level     += 2;
    elite.maxHealth  = elite.maxHealth * 160 / 100;
    elite.health     = elite.maxHealth;
    elite.attack     = elite.attack * 130 / 100 + 2;
    elite.defense   += 2;
    elite.xpReward   = elite.xpReward * 2;
    elite.goldReward = elite.goldReward * 2;

    /* Give it a second trick if it had none. */
    if (elite.special == SPECIAL_NONE)
        elite.special = SPECIAL_RAGE;
    if (elite.special2 == SPECIAL_NONE)
        elite.special2 = SPECIAL_DOUBLE;

    printf("\nA hush falls over the hall — this is no common foe.\n");
    printf("An elite enemy blocks your path: the %s!\n", elite.name);

    int result = startCombat(&player, &elite, 0);   /* no escape */

    if (result == 1)
    {
        printf("You bring the %s down. Its hoard is yours.\n", elite.name);
        grantMaterial(rand() % 6 == 0 ? MAT_RUNE : (rand() % MAT_COUNT), 1 + rand() % 2);
    }
    else
    {
        /* defeat is handled by the dungeon loop via player.health <= 0 */
        printf("The %s proves too much...\n", elite.name);
    }
}

/* EV_HAZARD: an environmental room. No combat — a small choice between
   the careful path (a modest, safe gain) and the risky path (a bigger
   gain or a bite of damage). */
void hazardRoom(int dungeonLevel)
{
    int kind = rand() % 3;
    int risk = 30 + dungeonLevel * 2;       /* deeper = nastier */
    if (risk > 70) risk = 70;

    if (kind == 0)
        printf("\nA collapsing bridge spans a black chasm.\n");
    else if (kind == 1)
        printf("\nThe hall ahead is flooded with cold, rising water.\n");
    else
        printf("\nThe air here is thick with a cursed, shimmering haze.\n");

    printf("1. Take the careful way (slow, but safe)\n");
    printf("2. Risk the quick way (you might get hurt, but the reward is better)\n");
    printf("Enter the number of your choice: ");

    if (readInt() == 2)
    {
        if (rand() % 100 < risk)
        {
            int dmg = 10 + rand() % 11;
            player.health -= dmg;
            if (player.health < 1)
                player.health = 1;
            printf("It goes wrong — you take %d damage scrambling clear.\n", dmg);
            showHealthBar();
        }
        else
        {
            printf("You make it through and grab something glinting on the far side!\n");
            player.gold += 60 + dungeonLevel * 5;
            grantMaterial(rand() % MAT_COUNT, 1 + rand() % 2);
        }
    }
    else
    {
        printf("You pick your way across without a scratch.\n");
        if (rand() % 2)
        {
            grantMaterial(rand() % 4, 1);
        }
        else
        {
            player.gold += 20;
            printf("You pocket 20 gold left behind by someone less lucky.\n");
        }
    }
}

/* EV_SHRINE: a quiet rest / boon node between fight clusters. */
void shrineRoom(int dungeonLevel)
{
    (void)dungeonLevel;

    printf("\nYou find a weathered shrine, still warm with old magic.\n");
    printf("1. Rest here (fully heal and restore mana)\n");
    printf("2. Leave an offering of 40 gold for a lasting blessing\n");
    printf("3. Pass it by\n");
    printf("Enter the number of your choice: ");

    switch (readInt())
    {
    case 1:
        healPlayer(&player, player.maxHealth);
        restoreMana(&player, player.maxMana);
        printf("You rest at the shrine. Your wounds close and your mind clears.\n");
        showHealthBar();
        break;

    case 2:
        if (player.gold >= 40)
        {
            player.gold -= 40;

            int boon = rand() % 3;
            if (boon == 0)
            {
                grantOffense(&player, 4);
            }
            else if (boon == 1)
            {
                player.defense += 4;
                printf("The shrine blesses your guard. Defense +4.\n");
            }
            else
            {
                player.maxHealth += 8;
                player.health += 8;
                printf("The shrine blesses your body. Max health +8.\n");
            }
        }
        else
            printf("You cannot spare the gold. The shrine stays silent.\n");
        break;

    default:
        printf("You leave the shrine undisturbed.\n");
        break;
    }
}

/* EV_BRANCH: a fork in the dungeon. The long way means another fight
   but richer loot; the quiet way skips ahead for a small, safe reward. */
void branchRoom(int dungeonLevel)
{
    printf("\nThe passage splits in two.\n");
    printf("1. The long way — more danger, but more to find\n");
    printf("2. The quiet way — slip ahead safely\n");
    printf("Enter the number of your choice: ");

    if (readInt() == 1)
    {
        printf("You take the long way, and something stirs in the dark...\n");

        Enemy foe;
        spawnMonster(&foe, dungeonLevel);
        int result = startCombat(&player, &foe, 1);

        if (result == 1)
        {
            printf("The long way pays off — a cache of loot!\n");
            player.gold += 80 + dungeonLevel * 5;
            grantMaterial(rand() % MAT_COUNT, 1 + rand() % 2);
            if (rand() % 2)
                player.potions++;
        }
    }
    else
    {
        printf("You take the quiet way and save your strength.\n");
        player.potions++;
        printf("You find a stray health potion along the shortcut.\n");
    }
}

/* ---------------------------------------------------------
   RUNNING ONE EVENT
   --------------------------------------------------------- */

int eventIsReady(int event)
{
    return (EVENTS_READY >> event) & 1;
}

void runEvent(int event, int dungeonLevel)
{
    switch (event)
    {
    case EV_MONSTER:    encounterMonster(dungeonLevel);  break;
    case EV_ITEM:       findItem();                      break;
    case EV_MERCHANT:   merchantEncounter();             break;
    case EV_FORAGE:     foragingOption();                break;
    case EV_DICE_TABLE: diceTable(dungeonLevel);         break;
    case EV_MINIBOSS:   minibossEncounter(dungeonLevel); break;
    case EV_STORY:      storyVignette(dungeonLevel);     break;
    case EV_FISHING:    fishingEvent(dungeonLevel);      break;
    case EV_MINE:       mineEvent(dungeonLevel);         break;
    case EV_CHEST:      chestEvent(dungeonLevel);        break;
    case EV_ARCHERY:    archeryEvent(dungeonLevel);      break;
    case EV_BLACKJACK:  blackjackTable(dungeonLevel);    break;
    case EV_NPC:        npcEncounter(dungeonLevel);      break;
    case EV_SECRET:     secretRoom(dungeonLevel);        break;
    case EV_OPTBOSS:    optionalBoss(dungeonLevel);      break;
    case EV_ELITE:      eliteEncounter(dungeonLevel);    break;
    case EV_HAZARD:     hazardRoom(dungeonLevel);        break;
    case EV_SHRINE:     shrineRoom(dungeonLevel);        break;
    case EV_BRANCH:     branchRoom(dungeonLevel);        break;
    default:            findItem();                      break;
    }
}
