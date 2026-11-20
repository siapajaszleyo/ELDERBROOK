/* =========================================================
   EPIC RPG ADVENTURE  (the expanded world)

   Build (everything in this folder):
       gcc -Wall -Wextra -o rpg *.c
   Run:
       ./rpg            (Windows: .\rpg.exe)
       ./rpg 42         (same random seed = same luck, for testing)

   FILES
     rpg.c      this file: the story and the flow of the game
     player.*   the hero's stats (ONE struct holds everything)
     combat.*   fights, level-ups, enemy specials
     monster.*  the monster roster (add new monsters here)
     chess.*    chess with a computer opponent
     snake.*    the snake minigame
     dice.*     the dice minigame
     save.*     saving, backups and loading
     input.*    safe keyboard reading
     game.h     shared settings (dungeon count, event types)
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "game.h"
#include "adventure.h"
#include "ui.h"
#include "world.h"
#include "events.h"
#include "hub.h"
#include "npc.h"
#include "secrets.h"
#include "endings.h"
#include "encounters.h"
#include "achievements.h"
#include "monster.h"
#include "player.h"
#include "combat.h"
#include "monster.h"
#include "chess.h"
#include "snake.h"
#include "dice.h"
#include "save.h"
#include "input.h"
#include "class.h"
#include "guild.h"
#include "finale.h"
#include "dungeonmod.h"
#include "ascii.h"
#include "tui.h"

#ifdef _WIN32
#include <windows.h>        /* SetConsoleOutputCP, for the heart symbols */
#endif

#define FIRST_KEY_DUNGEON   4     /* keys can appear from this dungeon on */
#define MAX_RIDDLE_TRIES    3

static int isCaster(void)
{
    return adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK;
}


/* =========================================================
   SAVING: checkpoints
   The game saves by itself at every checkpoint, so closing the window
   never loses more than the current fight. Typing q at a checkpoint
   quits cleanly; run the game again and choose Continue.
   ========================================================= */

static void autosave(void)
{
    if (!saveGame(&player, &progress))
        printf("[Warning: could not write the save file. Is the folder read-only?]\n");
}

/* Returns 1 if the player wants to stop for now. */
static int checkpoint(void)
{
    autosave();

    printf("\n[Progress saved]  Press Enter to continue, or type q to quit for now: ");

    char line[32];
    readOptional(line, sizeof line);

    return tolower((unsigned char)line[0]) == 'q';
}


/* Returns 1 = expedition completed, 0 = hero died, 2 = player quit for now */
static int runDungeon(int dungeonLevel)
{
    int justHadEncounter = 0;

    /* On fresh arrival, enforcers from any guild you've driven Hostile
       may ambush you on the road. */
    if (progress.planIndex == 0)
    {
        if (!guildAmbushCheck(dungeonLevel))
            return 0;
    }

    while (progress.planIndex < progress.planCount)
    {
        int event = progress.plan[progress.planIndex];

        runEvent(event, dungeonLevel);
        achievementCheck();

        if (player.health <= 0)
            return 0;

        progress.planIndex++;

        /* Now and then something unexpected happens on the road
           (never twice in a row, never right before the miniboss). */
        if (progress.planIndex < progress.planCount &&
            event != EV_MINIBOSS &&
            !justHadEncounter &&
            rand() % 100 < RANDOM_ENCOUNTER_PERCENT)
        {
            randomEncounter(dungeonLevel);

            if (player.health <= 0)
                return 0;

            justHadEncounter = 1;
        }
        else
            justHadEncounter = 0;

        /* A checkpoint between events (not after the final one) */
        if (progress.planIndex < progress.planCount)
            if (checkpoint())
                return 2;
    }

    return 1;
}

/* Endless floors reuse the fifteen region rosters and affixes, with enemy
   level scaling that continues beyond the campaign's final region. */
static int runEndlessDescent(void)
{
    for (;;)
    {
        int depth = adv.endlessDepth + 1;
        if (depth > 9999) depth = 9999;
        int locationLevel = (depth - 1) % DUNGEON_COUNT + 1;
        const Location *where = getLocation(locationLevel);
        Enemy enemy;

        printf("\n=================================\nENDLESS DESCENT — FLOOR %d\n=================================\n", depth);
        setDungeonMod(rollDungeonMod(locationLevel));
        announceDungeonMod();
        int kind = where->pool[rand() % where->poolCount];
        spawnMonsterOfKind(&enemy, kind, depth);
        printf("%s guards the descent.\n", enemy.name);

        int result = startCombat(&player, &enemy, 0);
        if (result != 1)
            return 0;

        adv.endlessDepth = depth;
        dungeonModOnClear(locationLevel);
        achievementCheck();
        autosave();
        printf("Floor %d cleared. 1. Descend  2. Return to Elderbrook: ", depth);
        if (readInt() != 1)
            return 1;
    }
}


/* =========================================================
   KEYS
   Each dungeon from 4 on may hold a key you don't have yet. If we
   are running out of dungeons, the last missing keys are guaranteed,
   so the game can always be finished.
   ========================================================= */

static int keysOwned(void)
{
    int count = 0;

    for (int i = 0; i < NUM_KEYS; i++)
        count += player.keys[i];

    return count;
}

static void collectKey(int keyType)
{
    const char *names[NUM_KEYS] = {"Fire", "Earth", "Air"};

    player.keys[keyType] = 1;
    printf("\nYou found the %s Key!\n", names[keyType]);
}

static void maybeFindKey(int dungeonLevel)
{
    if (dungeonLevel < FIRST_KEY_DUNGEON)
        return;

    int missing = NUM_KEYS - keysOwned();
    int dungeonsLeft = DUNGEON_COUNT - dungeonLevel + 1;

    if (missing > 0 && (dungeonsLeft <= missing || rand() % 2 == 0))
    {
        int pick = rand() % missing;

        for (int k = 0; k < NUM_KEYS; k++)
        {
            if (player.keys[k] == 0)
            {
                if (pick == 0)
                {
                    collectKey(k);
                    return;
                }
                pick--;
            }
        }
    }
}


/* =========================================================
   STORY
   ========================================================= */

/* Up to 3 tries (each miss costs 10 health, but never the last
   point). After that the guardian takes pity, so nobody gets stuck
   before the proposal. */
static void askRiddle(int dungeonLevel)
{
    const char *question;
    const char *choices = NULL;
    const char *answer;

    if (dungeonLevel == 1)
    {
        question = "when did we first meet ?\n";
        answer = "051224";
    }
    else if (dungeonLevel == 2)
    {
        question = "What's my favourite number? ";
        answer = "666";
    }
    else if (dungeonLevel == 3)
    {
        question = "Where did we meet? ";
        choices = "\nChoose your option wisely \t 1.Gym \t 2.Skatepark \t 3. A Random App ";
        answer = "3";
    }
    else
    {
        return;
    }

    printf("\nTo enter, answer this riddle: %s", question);
    if (choices != NULL)
        printf("%s", choices);

    for (int attempt = 1; attempt <= MAX_RIDDLE_TRIES; attempt++)
    {
        char text[64];

        printf("\nType in your answer here - ");
        readLine(text, sizeof text);

        if (strcmp(text, answer) == 0)
        {
            printf("Correct! You may enter the dungeon.\n");
            return;
        }

        printf("Incorrect!\n");
        player.health -= 10;
        if (player.health < 1)
            player.health = 1;
        printf("You lost 10 health for the wrong answer.\n");
        showHealthBar();

        if (attempt < MAX_RIDDLE_TRIES)
            printf("Think carefully and try again.\n");
    }

    printf("The guardian sighs, takes pity on you, and lets you through anyway.\n");
}

static void applyGateChoice(int choice)
{
    int attack = 0, defense = 0, health = 0, mana = 0;
    int crit = 0, initiative = 0, spell = 0;
    int agile = adv.playerClass == CLASS_ROGUE || adv.playerClass == CLASS_RANGER;

    if (choice == 1)
    {
        if (isCaster()) { mana = 8; health = -10; }
        else { attack = 5; crit = 8; defense = -4; }
    }
    else if (choice == 2)
    {
        if (isCaster()) { mana = 10; defense = -4; }
        else { crit = agile ? 14 : 8; initiative = agile ? 14 : 8; attack = -3; }
    }
    else if (isCaster()) { mana = 10; spell = -2; }
    else { defense = 10; initiative = -8; }

    if (player.critChance + crit > 100) crit = 100 - player.critChance;
    if (player.critChance + crit < 0) crit = -player.critChance;

    printf("\nYour expedition tradeoff: ");
    if (attack) printf("attack %s%d; ", attack > 0 ? "+" : "", attack);
    if (defense) printf("defense %s%d; ", defense > 0 ? "+" : "", defense);
    if (health) printf("max HP %s%d; ", health > 0 ? "+" : "", health);
    if (mana) printf("max MP %s%d; ", mana > 0 ? "+" : "", mana);
    if (crit) printf("critical chance %s%d%%; ", crit > 0 ? "+" : "", crit);
    if (initiative) printf("initiative %s%d; ", initiative > 0 ? "+" : "", initiative);
    if (spell) printf("spell power %s%d; ", spell > 0 ? "+" : "", spell);
    printf("for this expedition.\n");

    player.attack += attack;
    player.defense += defense;
    player.maxHealth += health;
    player.health += health;
    player.maxMana += mana;
    player.mana += mana;
    if (player.health > player.maxHealth) player.health = player.maxHealth;
    if (player.mana > player.maxMana) player.mana = player.maxMana;
    player.critChance += crit;
    if (player.critChance > 100) player.critChance = 100;
    if (player.critChance < 0) player.critChance = 0;
    adv.gateAttackDelta = attack;
    adv.gateDefenseDelta = defense;
    adv.gateHealthDelta = health;
    adv.gateManaDelta = mana;
    adv.gateCritDelta = crit;
    adv.gateInitiativeDelta = initiative;
    adv.gateSpellDelta = spell;
    adv.gateChoice = choice;
}

static void clearGateChoice(void)
{
    player.attack -= adv.gateAttackDelta;
    player.defense -= adv.gateDefenseDelta;
    player.maxHealth -= adv.gateHealthDelta;
    player.health -= adv.gateHealthDelta;
    player.maxMana -= adv.gateManaDelta;
    player.mana -= adv.gateManaDelta;
    player.critChance -= adv.gateCritDelta;
    if (player.critChance < 0) player.critChance = 0;
    if (player.critChance > 100) player.critChance = 100;
    if (player.health > player.maxHealth) player.health = player.maxHealth;
    if (player.health < 1) player.health = 1;
    if (player.mana > player.maxMana) player.mana = player.maxMana;
    adv.gateAttackDelta = adv.gateDefenseDelta = adv.gateHealthDelta = 0;
    adv.gateManaDelta = adv.gateCritDelta = adv.gateInitiativeDelta = 0;
    adv.gateSpellDelta = adv.gateChoice = 0;
}

/* Returns 1 = boss defeated, 0 = hero died, -1 = hero ran away.
   startCombat already pays out the XP and gold for a win.
   ids[]/count are the allied guilds whose boons weaken the boss. */
static int swordBossFight(const int *ids, int count)
{
    Enemy boss;

    initializeEnemy(&boss, "Final Boss", 17,
                    300,                        /* health      */
                    32,                         /* attack      */
                    9,                          /* defense     */
                    50 + player.level * 10,     /* xp reward   */
                    100 + player.level * 20,    /* gold reward */
                    SPECIAL_RAGE);
    boss.special2 = SPECIAL_DOUBLE;

    /* Arcanum wards away a trick, Wardens' volleys wound it in advance. */
    finaleApplyToBoss(&boss, ids, count);

    int result = startCombat(&player, &boss, 1);

    if (result == 1)
    {
        printf("\nYou defeated the Final Boss with your sword!\n");
        return 1;
    }

    if (result == 0)
    {
        printf("\nYou back away from the Final Boss... but he blocks the way onward.\n");
        return -1;
    }

    printf("\nYou were defeated by the Final Boss.\n");
    return 0;
}

/* Returns 1 if the boss is defeated, 0 if the hero dies.
   ids[]/count are the allied guilds that marched with you. */
static int fightFinalBoss(const int *ids, int count)
{
    printf("\nYou encounter the Final Boss!\n");

    while (1)
    {
        printf("\nHow will you face him?\n");
        printf("1. Battle with sword and spell\n");
        printf("2. Accept his challenge to a game of chess\n");
        printf("Enter the number of your choice: ");

        int choice = readInt();
        int won = 0;
        int rewardsAlreadyPaid = 0;

        if (choice == 1)
        {
            int outcome = swordBossFight(ids, count);

            if (outcome == 0)
            {
                printf("You have been slain by the Final Boss.\n");
                return 0;
            }

            if (outcome == 1)
            {
                won = 1;
                rewardsAlreadyPaid = 1;     /* the fight paid already */
            }
        }
        else if (choice == 2)
        {
            printf("\nChoose his skill (1 = Easy, 2 = Medium, 3 = Hard): ");
            int level = readInt();
            if (level < 1 || level > 3)
                level = 1;

            setChessOpponent("The Final Boss");
            int result = playChess(level);

            if (result == CHESS_WIN)
                won = 1;
            else if (result == CHESS_LOSE)
                printf("\nThe boss smirks. \"Again, or choose steel.\"\n");
            else if (result == CHESS_DRAW)
                printf("\nA draw. The boss demands a rematch.\n");
            else
                printf("\nYou step away from the board.\n");
        }
        else
        {
            printf("Invalid choice.\n");
        }

        if (won)
        {
            printf("You have defeated the Final Boss!\n");

            if (!rewardsAlreadyPaid)        /* chess wins pay out here */
            {
                player.gold += 100 + player.level * 20;
                gainXP(&player, 50 + player.level * 10);
            }
            return 1;
        }
    }
}

static void chooseKnight(void)
{
    printf("\nChoose the knight you wish to free:\n");
    printf("You have met these three on your journey, and they remember how you treated them.\n");
    printf("1. Sir Galahad - The Pure (High Defense)   [friendship %d/10]\n", npcAffinity(NPC_GALAHAD));
    printf("2. Lady Morgana - The Wise (High Mana)     [friendship %d/10]\n", npcAffinity(NPC_MORGANA));
    printf("3. Sir Batman - The Brave (High Attack)    [friendship %d/10]\n", npcAffinity(NPC_BATMAN));

    while (1)
    {
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            player.knightChosen = 1;
            printf("You chose Sir Galahad - The Pure.\n");
            player.defense += 20;
            return;

        case 2:
            player.knightChosen = 2;
            printf("You chose Lady Morgana - The Wise.\n");
            player.maxMana += 20;
            player.mana += 20;
            return;

        case 3:
            player.knightChosen = 3;
            printf("You chose Sir Batman - The Brave.\n");
            if (isCaster())
                printf("\nBatman's resolve strengthens your magical reserves.\n");
            else
                printf("\nBatman shares the edge of his weapon with you.\n");
            grantOffense(&player, 20);
            return;

        default:
            printf("Invalid choice.\n");
        }
    }
}

static void endGame(void)
{
    printf("\nThank you for playing! %s's adventure ends here.\n", player.name);
    printf("Dungeons Cleared: %d\n", progress.dungeonsCleared);
    printf("Final Level: %d (%s)\n", player.level, rankName(player.level));
    printf("Gold Collected: %d\n", player.gold);
    printSummary();
}


/* =========================================================
   THE MAIN LOOP OF THE ADVENTURE
   ========================================================= */

static void gameOver(void)
{
    printf("\nYou have fallen in battle. Game over.\n");
    printf("Your last checkpoint is saved: run the game again and choose Continue to try again.\n");
}

/* Set up a trip to a location. Returns 0 if the player chose to save and
   quit at the gate instead. */
static int enterLocation(int level, int revisit)
{
    const Location *where = getLocation(level);

#ifdef RPG_TUI
    asciiLocationArt(level);
#endif

    adv.vowUsed = adv.loanUsed = adv.darkUsed = 0;
    if (adv.affinity[NPC_GALAHAD] >= 12) adv.vowReady = 1;
    if (adv.affinity[NPC_MORGANA] >= 12)
    {
        adv.loanReady = 1;
        adv.loanMana = isCaster() ? 6 : 0;
        adv.loanShield = isCaster() ? 0 : 12;
    }
    if (adv.affinity[NPC_BATMAN] >= 12) adv.darkReady = 1;

    if (revisit)
    {
        printf("\nYou return to Dungeon %d: %s...\n", level, where->name);
    }
    else
    {
        printf("\nYou approach Dungeon %d: %s...\n", level, where->name);
        showStats();
        autosave();

        printf("\nYou stand before the dungeon gate. How do you want to enter?\n");
        if (isCaster())
        {
            printf("1. Charge: +8 max MP, -10 max HP this expedition.\n");
            printf("2. Sneak: +10 max MP, -4 defense this expedition.\n");
            printf("3. Wait: +10 max MP, -2 spell power this expedition.\n");
        }
        else if (adv.playerClass == CLASS_ROGUE || adv.playerClass == CLASS_RANGER)
        {
            printf("1. Charge: +5 attack, +%d%% crit, -4 defense this expedition.\n", player.critChance + 8 > 100 ? 100 - player.critChance : 8);
            printf("2. Sneak: +%d%% crit, +14 initiative, -3 attack this expedition.\n", player.critChance + 14 > 100 ? 100 - player.critChance : 14);
            printf("3. Wait: +10 defense, -8 initiative this expedition.\n");
        }
        else
        {
            printf("1. Charge: +5 attack, +%d%% crit, -4 defense this expedition.\n", player.critChance + 8 > 100 ? 100 - player.critChance : 8);
            printf("2. Sneak: +%d%% crit, +8 initiative, -3 attack this expedition.\n", player.critChance + 8 > 100 ? 100 - player.critChance : 8);
            printf("3. Wait: +10 defense, -8 initiative this expedition.\n");
        }
        printf("4. Save and quit for now.\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            printf("\nYou charge into the dungeon fearlessly!\n");
            applyGateChoice(1);
            break;
        case 2:
            printf("\nYou sneak into the dungeon, hoping to avoid enemies.\n");
            applyGateChoice(2);
            break;
        case 3:
            printf("\nYou wait outside the dungeon, watching for any signs of danger...\n");
            applyGateChoice(3);
            break;
        case 4:
            autosave();
            return 0;
        default:
            printf("\nInvalid choice. You proceed cautiously.\n");
        }

        askRiddle(level);
    }

    printf("\n%s\n", where->arrival);
    adv.visited[level - 1] = 1;

    /* Phase 5b: roll this run's modifier before the plan is built, so it
       can bias the layout. Revisits run plain. */
    if (revisit)
        setDungeonMod(DMOD_NONE);
    else
    {
        rollDungeonMod(level);
        announceDungeonMod();
    }

    buildPlanFor(&progress, level, revisit);
    progress.currentDungeon = level;
    autosave();

    return 1;
}

static void startGame(void)
{
    while (1)
    {
        int level;
        int revisit;

        if (progress.currentDungeon == 0)
        {
            /* In town: shop, talk, train, then choose where to go */
            int destination = hubVisit();

            if (destination == HUB_QUIT)
            {
                printf("\nGame saved. See you soon, %s!\n", player.name);
                return;
            }

            if (destination == HUB_FINAL)
                break;

            if (destination == HUB_ENDLESS)
            {
                if (!runEndlessDescent())
                    gameOver();
                continue;
            }

            level = destination;
            revisit = level <= progress.dungeonsCleared;

            if (!enterLocation(level, revisit))
            {
                printf("\nGame saved. See you soon, %s!\n", player.name);
                return;
            }
        }
        else
        {
            /* Continuing a saved expedition */
            level = progress.currentDungeon;
            revisit = progress.revisit;
            printf("\nYou return to Dungeon %d...\n", level);
        }

        int outcome = runDungeon(level);

        if (outcome == 0)
        {
            gameOver();
            return;
        }

        if (outcome == 2)
        {
            printf("\nGame saved. See you soon, %s!\n", player.name);
            return;
        }

        if (revisit)
        {
            printf("\nYou finish exploring %s and head back to town.\n", getLocation(level)->name);
        }
        else
        {
            printf("\n*** Dungeon %d cleared! ***\n", level);
            clearGateChoice();
            dungeonModOnClear(level);
            maybeFindKey(level);
            progress.dungeonsCleared++;
        }

        progress.currentDungeon = 0;
        progress.planCount = 0;
        progress.planIndex = 0;
        progress.revisit = 0;

        achievementCheck();
    }

    /* The player chose to face the final boss from the world map */
    player.health = player.maxHealth;
    player.mana = player.maxMana;
    printf("\nYou rest one last time before the final battle and recover your strength.\n");
    autosave();

    if (keysOwned() != NUM_KEYS)
    {
        printf("\nYou did not collect all the elemental keys. You cannot face the final boss.\n");
        return;
    }

    /* ---- Phase 5a: the faction muster ----
       First the guilds you brought to Allied standing muster at the gate
       and lend their boons; then you pick a knight to free; then the
       battle; then your allies spend themselves for the final blow; then
       the bittersweet epilogue. */
    int allyIds[GUILD_COUNT];
    int allyCount = finaleMuster(allyIds);

    chooseKnight();

    if (!fightFinalBoss(allyIds, allyCount))
    {
        gameOver();
        return;
    }

    finaleFalls(allyIds, allyCount);
    printEpilogue(allyIds, allyCount);
    endGame();

    deleteSave();       /* the adventure is finished */
}


/* =========================================================
   MAIN: where every C program begins
   ========================================================= */

int main(int argc, char *argv[])
{
    unsigned int seed = (unsigned int)time(NULL);

    if (argc > 1)
        seed = (unsigned int)strtoul(argv[1], NULL, 10);

    srand(seed);

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);     /* so the heart symbols draw correctly */
#endif

#ifdef RPG_TUI
    tuiInit();
    atexit(tuiShutdown);
#endif

    asciiTitleScreen();
#ifdef RPG_TUI
    tuiColor(TUI_CYAN);
    printf("  Launching in TUI mode (foot / modern terminal recommended).\n");
    tuiReset();
#endif
    printf("  Press Enter to begin... ");
    fflush(stdout);
    {
        char line[8];
        readOptional(line, sizeof line);
    }
#ifdef RPG_TUI
    tuiClear();
#endif

    memset(&progress, 0, sizeof progress);
    initializeAdventure();

    printf("Welcome to the Epic RPG Adventure!\n");

    if (saveExists())
    {
        printf("\nA saved adventure was found.\n");
        printf("1. Continue it\n");
        printf("2. Start a new game\n");
        printf("Enter the number of your choice: ");

        if (readInt() == 1)
        {
            int loaded = loadGame(&player, &progress);

            if (loaded == 0)
            {
                printf("The save file could not be read. Starting a new game.\n");
            }
            else
            {
                if (loaded == 2)
                    printf("The main save was damaged, so the backup was loaded.\n");

                printf("\nWelcome back, %s!\n", player.name);
                startGame();
                return 0;
            }
        }
    }

    printf("Enter your name, brave adventurer: ");

    char name[50];
    readLine(name, sizeof name);

    initializePlayer(&player, name);
    initializeAdventure();
    memset(&progress, 0, sizeof progress);

    /* ---- Character 2.0: Class & Discipline Selection ---- */
    printf("\nBefore you set out, you must choose who you are.\n");
    ClassKind chosenClass = chooseClassScreen();
    Discipline chosenDisc = chooseDisciplineScreen(chosenClass);

    adv.playerClass = chosenClass;
    adv.playerDiscipline = chosenDisc;
    player.playerClass = chosenClass;
    player.playerDiscipline = chosenDisc;

    applyClassLoadout(chosenClass, chosenDisc);

    printf("\nYou are a %s of the %s discipline.\n",
           className(chosenClass), disciplineName(chosenDisc));
    printf("\nChoose a difficulty: 1. Story  2. Standard  3. Veteran: ");
    int difficulty = readInt();
    if (difficulty < 1 || difficulty > 3) difficulty = 2;
    adv.difficulty = difficulty - 1;
    printf("Difficulty set to %s.\n",
           adv.difficulty == 0 ? "Story" : adv.difficulty == 1 ? "Standard" : "Veteran");
    printf("Your journey begins...\n");
    /* ---- End Character 2.0 ---- */

    printf("You are %s, on a quest to choose a worthy knight.\n", player.name);
    printf("Press 's' to start your quest: ");

    char answer[16];
    readLine(answer, sizeof answer);

    if (tolower((unsigned char)answer[0]) == 's')
    {
        printf("\nYou embark on your quest...\n");
        startGame();
    }
    else
        printf("Invalid choice. Exiting game.\n");

    return 0;
}
