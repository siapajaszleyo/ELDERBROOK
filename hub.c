#include <stdio.h>
#include <string.h>

#include "hub.h"
#include "adventure.h"
#include "ui.h"
#include "items.h"
#include "skills.h"
#include "npc.h"
#include "quest.h"
#include "achievements.h"
#include "events.h"
#include "world.h"
#include "secrets.h"
#include "save.h"
#include "input.h"
#include "archery.h"
#include "memory.h"
#include "reflex.h"
#include "lockpick.h"
#include "blackjack.h"
#include "dice.h"
#include "snake.h"
#include "chess.h"
#include "class.h"
#include "balance.h"
#include "path.h"
#include "forge.h"
#include "ascii.h"
#include "tui.h"

static void saveNow(void)
{
    if (!saveGame(&player, &progress))
        printf("[Warning: could not write the save file. Is the folder read-only?]\n");
}

static void printMaterials(void)
{
    printf("Materials: ");

    for (int m = 0; m < MAT_COUNT; m++)
        printf("%s %d%s", materialName(m), adv.materials[m], m < MAT_COUNT - 1 ? ", " : "\n");
}

/* A "(!)" marker when someone has a quest waiting to be handed in */
static const char *flag(int npc)
{
    return (npcAvailable(npc) && questReadyFor(npc)) ? " (!)" : "";
}


/* =========================================================
   THE BLACKSMITH
   ========================================================= */

static void blacksmith(void)
{
    if (!adv.npcMet[NPC_BROKK])
        talkTo(NPC_BROKK);

    while (1)
    {
        int swordLeft = MAX_SWORD_UPGRADES - player.swordUpgrades;
        int armorLeft = 3 - adv.flags[FLAG_ARMOR_UPGRADES];

        printf("\n=================================\n");
        printf("         BROKK'S FORGE\n");
        printf("=================================\n");
        printf("Gold: %d\n", player.gold);
        printMaterials();
        showEquipment();

        printf("\n1. Craft equipment and gadgets\n");
        printf("2. Change equipment\n");
        printf("3. Sharpen your sword (+10 attack, 100 gold, %d left)\n", swordLeft);
        printf("4. Reinforce your armor (+5 defense, 8 Iron + 60 gold, %d left)\n", armorLeft);
        printf("5. Enchanting (bind runes to your gear)\n");
        printf("6. Relic work (Covenant craft)\n");
        printf("7. Talk to Brokk%s\n", flag(NPC_BROKK));
        printf("8. Leave\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            craftingMenu();
            achievementCheck();
            break;

        case 2:
            equipmentMenu();
            break;

        case 3:
            if (swordLeft <= 0)
                printf("\"I can't make that blade any sharper,\" Brokk says.\n");
            else if (player.gold < 100)
                printf("Not enough gold!\n");
            else
            {
                player.gold -= 100;
                grantOffense(&player, 10);
                player.swordUpgrades++;
                printf("Brokk hones your blade. Attack power increased!\n");
            }
            break;

        case 4:
            if (adv.armor == 0)
                printf("\"You're not wearing any armour,\" Brokk says.\n");
            else if (armorLeft <= 0)
                printf("\"That's as strong as I can make it.\"\n");
            else if (player.gold < 60 || adv.materials[MAT_IRON] < 8)
                printf("You need 8 Iron and 60 gold.\n");
            else
            {
                player.gold -= 60;
                adv.materials[MAT_IRON] -= 8;
                player.defense += 5;
                adv.flags[FLAG_ARMOR_UPGRADES]++;
                printf("Brokk reinforces your armour. Defense increased!\n");
            }
            break;

        case 5:
            forgeEnchantMenu();
            achievementCheck();
            break;

        case 6:
            forgeRelicMenu();
            break;

        case 7:
            talkTo(NPC_BROKK);
            break;

        case 8:
            return;

        default:
            printf("Invalid choice.\n");
        }
    }
}


/* =========================================================
   THE TAVERN
   ========================================================= */

static void rumors(void)
{
    int shown = 0;

    printf("\nMarla leans in and lowers her voice...\n");

    for (int level = 1; level <= DUNGEON_COUNT && shown < 3; level++)
    {
        const Location *where = getLocation(level);

        if (!adv.visited[level - 1])
            continue;

        if (where->secret >= 0 && !adv.secrets[where->secret])
        {
            printf("\"There's more to %s than meets the eye. Something is hidden there.\"\n", where->name);
            shown++;
        }
        else if (where->optBoss >= 0 && !adv.optBoss[where->optBoss] &&
                 optBossUnlocked(where->optBoss))
        {
            printf("\"A way has opened to something terrible in %s... if you're brave.\"\n", where->name);
            shown++;
        }
    }

    if (shown < 3 && adv.stats[ST_LEGEND_FISH] == 0)
    {
        printf("\"Old Pete swears a golden koi lives where the water runs deepest.\"\n");
        shown++;
    }

    if (shown == 0)
        printf("\"Nothing new tonight. Come back after your next trip.\"\n");
}

static void gamblingTables(void)
{
    int level = progress.dungeonsCleared < 1 ? 1 : progress.dungeonsCleared;

    printf("\n1. Dice table\n2. Blackjack table\n3. Back\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if (choice == 1)
        diceTable(level);
    else if (choice == 2)
        blackjackTable(level);
}

static void talkToMenu(const int *folk, int count)
{
    int list[NPC_COUNT];
    int n = 0;

    printf("\nWho do you want to talk to?\n");

    for (int i = 0; i < count; i++)
    {
        if (!npcAvailable(folk[i]))
            continue;

        list[n++] = folk[i];
        printf("%d. %s%s\n", n, npcName(folk[i]), flag(folk[i]));
    }

    if (n == 0)
    {
        printf("Nobody is here right now.\n");
        return;
    }

    printf("0. Nobody\nEnter the number of your choice: ");

    int choice = readInt();

    if (choice >= 1 && choice <= n)
        talkTo(list[choice - 1]);
}

/* Paid healing — the real gold sink that replaces the old free inn
   full-heal. The innkeeper patches you up for gold. */
static void innkeeperHealing(void)
{
    while (1)
    {
        int missHp = player.maxHealth - player.health;
        int missMp = player.maxMana   - player.mana;
        int fullCost = missHp * TAVERN_HEAL_HP_COST + missMp * TAVERN_HEAL_MP_COST;
        fullCost = fullCost - fullCost * TAVERN_FULL_HEAL_DISCOUNT / 100;

        printf("\n--- The innkeeper's rooms ---\n");
        printf("HP: %d/%d   MP: %d/%d   Gold: %d\n",
               player.health, player.maxHealth, player.mana, player.maxMana, player.gold);
        printf("\n1. Full rest (restore everything) — %d gold\n", fullCost);
        printf("2. Patch wounds (restore some HP) — %d gold per 1 HP\n", TAVERN_HEAL_HP_COST);
        printf("3. Clear your head (restore some MP) — %d gold per 1 MP\n", TAVERN_HEAL_MP_COST);
        printf("0. Back\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            if (missHp == 0 && missMp == 0) { printf("You are already at full strength.\n"); break; }
            if (player.gold < fullCost) { printf("Not enough gold.\n"); break; }
            player.gold   -= fullCost;
            player.health  = player.maxHealth;
            player.mana    = player.maxMana;
            printf("You sleep in a warm bed and wake fully restored.\n");
            break;
        case 2:
        {
            if (missHp == 0) { printf("Your wounds are already closed.\n"); break; }
            printf("Restore how much HP? (max %d): ", missHp);
            int amt = readInt();
            if (amt < 1) break;
            if (amt > missHp) amt = missHp;
            int cost = amt * TAVERN_HEAL_HP_COST;
            if (player.gold < cost) { printf("Not enough gold.\n"); break; }
            player.gold   -= cost;
            player.health += amt;
            printf("The innkeeper tends your wounds. +%d HP for %d gold.\n", amt, cost);
            break;
        }
        case 3:
        {
            if (missMp == 0) { printf("Your mind is already clear.\n"); break; }
            printf("Restore how much MP? (max %d): ", missMp);
            int amt = readInt();
            if (amt < 1) break;
            if (amt > missMp) amt = missMp;
            int cost = amt * TAVERN_HEAL_MP_COST;
            if (player.gold < cost) { printf("Not enough gold.\n"); break; }
            player.gold -= cost;
            player.mana += amt;
            printf("A hot drink and a quiet corner. +%d MP for %d gold.\n", amt, cost);
            break;
        }
        case 0: return;
        default: printf("Invalid choice.\n");
        }
    }
}

static void tavern(void)
{
    static const int folk[] = { NPC_MARLA, NPC_FINN, NPC_PETE, NPC_ZARA,
                                NPC_ALDRIC, NPC_WANDERER, NPC_MORGANA };

    while (1)
    {
        printf("\n=================================\n");
        printf("      THE SLEEPING GRIFFIN (tavern)\n");
        printf("=================================\n");
        printf("A fire crackles. The room smells of stew and old stories.\n");
        printf("\n1. Talk to someone\n");
        printf("2. Quest journal\n");
        printf("3. Ask Marla for rumors\n");
        printf("4. Gambling tables\n");
        printf("5. Rent a room (rest / heal for gold)\n");
        printf("6. Leave\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1: talkToMenu(folk, (int)(sizeof folk / sizeof folk[0])); break;
        case 2: questJournal();                break;
        case 3: rumors();                      break;
        case 4: gamblingTables(); achievementCheck(); break;
        case 5: innkeeperHealing();            break;
        case 6: return;
        default: printf("Invalid choice.\n");
        }
    }
}


/* =========================================================
   THE MERCHANT
   ========================================================= */

static void sellMaterials(void)
{
    static const int price[MAT_COUNT] = { 8, 5, 6, 20, 60, 30, 100 };

    printf("\nTobias will buy your materials:\n");
    for (int m = 0; m < MAT_COUNT; m++)
        printf("%d. %-15s (you have %d)  %d gold each\n", m + 1, materialName(m), adv.materials[m], price[m]);
    printf("0. Back\nEnter the number of your choice: ");

    int choice = readInt();

    if (choice < 1 || choice > MAT_COUNT)
        return;

    int m = choice - 1;

    printf("Sell how many %s? (0 to cancel): ", materialName(m));

    int amount = readInt();

    if (amount < 1)
        return;
    if (amount > adv.materials[m])
        amount = adv.materials[m];

    adv.materials[m] -= amount;
    player.gold += amount * price[m];
    printf("Sold %d for %d gold.\n", amount, amount * price[m]);
}

static void buyMaterials(void)
{
    static const int mat[3]   = { MAT_IRON, MAT_WOOD, MAT_HERB };
    static const int cost[3]  = { 25, 15, 15 };

    printf("\n");
    for (int i = 0; i < 3; i++)
        printf("%d. 1 %s (%d gold)\n", i + 1, materialName(mat[i]), cost[i]);
    printf("0. Back\nEnter the number of your choice: ");

    int choice = readInt();

    if (choice < 1 || choice > 3)
        return;

    if (player.gold < cost[choice - 1])
        printf("Not enough gold!\n");
    else
    {
        player.gold -= cost[choice - 1];
        adv.materials[mat[choice - 1]]++;
        printf("You buy 1 %s.\n", materialName(mat[choice - 1]));
    }
}

static void buyItem(int cost, int *counter, const char *name)
{
    if (player.gold < cost)
    {
        printf("Not enough gold!\n");
        return;
    }

    player.gold -= cost;
    (*counter)++;
    printf("You buy a %s.\n", name);
}

static void merchant(void)
{
    if (!adv.npcMet[NPC_TOBIAS])
        talkTo(NPC_TOBIAS);

    while (1)
    {
        printf("\n=================================\n");
        printf("      TOBIAS'S EMPORIUM\n");
        printf("=================================\n");
        printf("Gold: %d\n", player.gold);
        printf("1. Health Potion (50)      4. Fire Bomb (60)\n");
        printf("2. Mana Potion (30)        5. Smoke Bomb (50)\n");
        printf("3. Elixir, full heal (150) 6. Sell materials\n");
        printf("7. Buy materials           8. Talk to Tobias\n");
        printf("9. Leave\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1: buyItem(50,  &player.potions,     "health potion"); break;
        case 2: buyItem(30,  &player.manaPotions, "mana potion");   break;
        case 3: buyItem(150, &player.elixirs,     "elixir");        break;
        case 4: buyItem(60,  &player.fireBombs,   "fire bomb");     break;
        case 5: buyItem(50,  &player.smokeBombs,  "smoke bomb");    break;
        case 6: sellMaterials();       break;
        case 7: buyMaterials();        break;
        case 8: talkTo(NPC_TOBIAS);    break;
        case 9: return;
        default: printf("Invalid choice.\n");
        }
    }
}


/* =========================================================
   THE TRAINING AREA
   Three trials, each with three tiers. Each tier can be passed once
   and gives a permanent bonus. You can also practise any minigame
   with nothing at stake.
   ========================================================= */

static void trialReward(int trial, int tier)
{
    /* trial 0 = archery, 1 = reflex, 2 = memory */
    switch (trial * 3 + tier)
    {
    case 0: player.maxHealth += 5; player.health += 5; printf("Reward: +5 maximum health!\n"); break;
    case 1: grantOffense(&player, 3); break;
    case 2: player.critChance += 3; if (player.critChance > 100) player.critChance = 100; printf("Reward: +3%% critical chance!\n"); break;
    case 3: player.maxMana += 5; player.mana += 5; printf("Reward: +5 maximum mana!\n"); break;
    case 4: player.defense += 2;     printf("Reward: +2 defense!\n"); break;
    case 5: player.maxHealth += 5; player.health += 5; printf("Reward: +5 maximum health!\n"); break;
    case 6: player.skillPoints++;    printf("Reward: +1 skill point!\n"); break;
    case 7: player.maxMana += 5; player.mana += 5; printf("Reward: +5 maximum mana!\n"); break;
    case 8: grantOffense(&player, 3); break;
    }
}

static void trial(int which)
{
    static const char *names[3] = { "Archery", "Reflex", "Memory" };
    static const int levels[3] = { 1, 6, 11 };
    static const int archeryNeeded[3] = { 25, 35, 42 };

    int tier = adv.training[which];

    if (tier >= 3)
    {
        printf("You have passed every %s trial.\n", names[which]);
        return;
    }

    printf("\n%s trial, tier %d of 3.\n", names[which], tier + 1);

    int passed = 0;

    if (which == 0)
    {
        printf("Score at least %d to pass.\n", archeryNeeded[tier]);
        passed = playArchery(levels[tier]) >= archeryNeeded[tier];
    }
    else if (which == 1)
        passed = playReflex(levels[tier], "The trainer") == MINI_WIN;
    else
        passed = playMemory(levels[tier], "The trainer") == MINI_WIN;

    if (!passed)
    {
        printf("Not this time. Try again whenever you like.\n");
        return;
    }

    printf("\nTRIAL PASSED!\n");
    trialReward(which, tier);
    adv.training[which]++;
}

static void practice(void)
{
    printf("\nPractice with nothing at stake (no rewards):\n");
    printf("1. Archery   2. Lockpicking   3. Memory   4. Reflex\n");
    printf("5. Blackjack 6. Dice          7. Snake    8. Chess (easy)\n");
    printf("0. Back\nEnter the number of your choice: ");

    switch (readInt())
    {
    case 1: playArchery(1);                        break;
    case 2: playLockpick(1, "a practice lock");    break;
    case 3: playMemory(1, "The trainer");          break;
    case 4: playReflex(1, "The trainer");          break;
    case 5: playBlackjackDuel("The trainer");      break;
    case 6: playDice(1, "The trainer");            break;
    case 7: playSnake(1, "The trainer");           break;
    case 8: setChessOpponent("The trainer"); playChess(1); break;
    default: break;
    }
}

static void classTrainer(void)
{
    Discipline d = (Discipline)adv.playerDiscipline;

    if (adv.playerClass == CLASS_NONE || d == DISC_NONE)
    {
        printf("\nYou have not chosen a class and discipline yet.\n");
        return;
    }

    int base = (d - 1) * 3;   /* ability index of tier 1 */

    while (1)
    {
        printf("\n=================================\n");
        printf("      CLASS ABILITY TRAINER\n");
        printf("=================================\n");
        printf("%s / %s\n", className((ClassKind)adv.playerClass), disciplineName(d));
        printf("Skill points: %d   (unlocking a tier costs 1)\n\n", player.skillPoints);

        for (int t = 0; t < 3; t++)
        {
            int ab = base + t;
            int known = adv.classAbilities[ab];
            int prereq = (t == 0) || adv.classAbilities[base + t - 1];
            const char *state = known ? "[learned]"
                              : prereq ? "[available]"
                              : "[locked]";

            printf("%d. Tier %d: %-18s %-11s\n", t + 1, t + 1,
                   abilityName(ab), state);
            printf("     %s\n", abilityDescription(ab));
        }

        printf("\nEnter a tier number to unlock it, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > 3)
        {
            printf("Invalid choice.\n");
            continue;
        }

        int t = choice - 1;
        int ab = base + t;

        if (adv.classAbilities[ab])
            printf("You already know %s.\n", abilityName(ab));
        else if (t > 0 && !adv.classAbilities[base + t - 1])
            printf("Learn %s first.\n", abilityName(base + t - 1));
        else if (player.skillPoints < 1)
            printf("You have no skill points.\n");
        else
        {
            player.skillPoints--;
            adv.classAbilities[ab] = 1;
            printf("You have mastered %s!\n", abilityName(ab));
        }
    }
}


static void training(void)
{
    static const int folk[] = { NPC_SERAPHINE, NPC_BATMAN, NPC_GALAHAD };

    while (1)
    {
        printf("\n=================================\n");
        printf("        TRAINING AREA\n");
        printf("=================================\n");
        printf("Skill points: %d   Trials passed: archery %d/3, reflex %d/3, memory %d/3\n",
               player.skillPoints, adv.training[0], adv.training[1], adv.training[2]);
        printf("\n1. Skill tree\n");
        printf("2. Class abilities\n");
        printf("3. Archery trial\n");
        printf("4. Reflex trial\n");
        printf("5. Memory trial\n");
        printf("6. Practice a minigame\n");
        printf("7. Talk to the people here\n");
        printf("8. Leave\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1: skillMenu();                       break;
        case 2: classTrainer();                    break;
        case 3: trial(0); achievementCheck();      break;
        case 4: trial(1); achievementCheck();      break;
        case 5: trial(2); achievementCheck();      break;
        case 6: practice();                        break;
        case 7: talkToMenu(folk, 3);               break;
        case 8: return;
        default: printf("Invalid choice.\n");
        }
    }
}


/* =========================================================
   THE WORLD MAP
   ========================================================= */

static int keysComplete(void)
{
    return player.keys[0] && player.keys[1] && player.keys[2];
}

/* Returns 0 for "back", 1..15 for a location, HUB_FINAL for the finale */
static int worldMap(void)
{
    while (1)
    {
#ifdef RPG_TUI
        asciiWorldMapHeader();
#endif
        printf("\n=================================\n");
        printf("          WORLD MAP\n");
        printf("=================================\n");

        for (int level = 1; level <= DUNGEON_COUNT; level++)
        {
            GuildId gh = guildAtLocation(level);
            char tag[48];
            tag[0] = '\0';
            if (gh < GUILD_COUNT)
                snprintf(tag, sizeof tag, "  <Guild: %s>", guildName(gh));

            if (level <= progress.dungeonsCleared)
                printf("%2d. %-26s [cleared]%s\n",
                       level, getLocation(level)->name, tag);
            else if (level == progress.dungeonsCleared + 1)
                printf("%2d. %-26s [NEXT]%s\n", level, getLocation(level)->name, tag);
            else
                printf("%2d. ???                        [locked]\n", level);
        }

        if (progress.dungeonsCleared >= DUNGEON_COUNT)
        {
            printf("16. The Final Boss               %s\n",
                   keysComplete() ? "[the summit awaits]" : "[you need all three elemental keys]");
            printf("17. Endless Descent              [deepest floor: %d]\n", adv.endlessDepth);
        }

        showKeys();
        printf("0. Back to town\nEnter a number to travel: ");

        int choice = readInt();

        if (choice == 0)
            return 0;

        if (choice == HUB_FINAL && progress.dungeonsCleared >= DUNGEON_COUNT)
        {
            if (keysComplete())
                return HUB_FINAL;

            printf("The summit door will not open without the Fire, Earth and Air keys.\n");
            continue;
        }

        if (choice == HUB_ENDLESS && progress.dungeonsCleared >= DUNGEON_COUNT)
            return HUB_ENDLESS;

        if (choice >= 1 && choice <= DUNGEON_COUNT)
        {
            if (choice > progress.dungeonsCleared + 1)
            {
                printf("That road isn't open yet. Clear the locations before it first.\n");
                continue;
            }

            /* A guild keeps its house here — you may visit it or explore. */
            GuildId gh = guildAtLocation(choice);
            if (gh < GUILD_COUNT)
            {
                printf("\n%s is home to the %s.\n",
                       getLocation(choice)->name, guildName(gh));
                printf("1. Visit the guild house\n");
                printf("2. Travel there to explore / fight\n");
                printf("0. Back\nEnter the number of your choice: ");
                int gchoice = readInt();
                if (gchoice == 1) { guildVisit(choice); continue; }
                if (gchoice == 2) return choice;
                continue;
            }

            return choice;
        }

        printf("Invalid choice.\n");
    }
}


/* =========================================================
   THE ADVENTURER'S PACK
   ========================================================= */

static void pack(void)
{
    while (1)
    {
        printf("\n1. Stats and equipment\n2. Change equipment\n3. Quest journal\n");
        printf("4. Achievements\n5. Guild ledger\n6. Back\nEnter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            showStats();
            pathShow(&player);
            showEquipment();
            printMaterials();
            printf("Fire bombs: %d   Smoke bombs: %d   Elixirs: %d   Skill points: %d\n",
                   player.fireBombs, player.smokeBombs, player.elixirs, player.skillPoints);
            printf("Critical chance: %d%%\n", player.critChance);
            break;
        case 2: equipmentMenu();   break;
        case 3: questJournal();    break;
        case 4: showAchievements(); break;
        case 5: guildLedger();     break;
        case 6: return;
        default: printf("Invalid choice.\n");
        }
    }
}


/* =========================================================
   THE TOWN
   ========================================================= */

int hubVisit(void)
{
#ifdef RPG_TUI
    asciiTownGate();
#endif
    if (!adv.flags[FLAG_INTRO_DONE])
    {
        adv.flags[FLAG_INTRO_DONE] = 1;
        printf("\n=================================\n");
        printf("   ELDERBROOK, the last safe town\n");
        printf("=================================\n");
        printf("Lanterns glow along a muddy street. A smith, a tavern, a market and a\n");
        printf("training yard huddle against the wind. Everyone here has heard of the\n");
        printf("fifteen dangerous places beyond the walls. Some have even been there.\n");
    }

    /* Coming home gives only a small free rest now; a full restore is a
       paid service at the tavern (see "Rent a room"). This turns healing
       into a real gold sink instead of a free top-up every trip. */
    {
        int freeHp = player.maxHealth * INN_FREE_REST_PERCENT / 100;
        int freeMp = player.maxMana   * INN_FREE_REST_PERCENT / 100;
        player.health += freeHp;
        player.mana   += freeMp;
        if (player.health > player.maxHealth) player.health = player.maxHealth;
        if (player.mana   > player.maxMana)   player.mana   = player.maxMana;
        printf("\nYou catch a little rest by the gate (+%d HP, +%d MP).\n", freeHp, freeMp);
        printf("For a full recovery, rent a room at the tavern.\n");
    }

    achievementCheck();
    saveNow();

    while (1)
    {
#ifdef RPG_TUI
        tuiStatusStrip(player.name, player.level, player.health, player.maxHealth,
                       player.mana, player.maxMana, player.gold,
                       progress.dungeonsCleared, DUNGEON_COUNT);
#endif
        int training_flag = questReadyFor(NPC_SERAPHINE) || questReadyFor(NPC_BATMAN) || questReadyFor(NPC_GALAHAD);
        int tavern_flag = 0;

        for (int n = 0; n < NPC_COUNT; n++)
            if (n != NPC_BROKK && n != NPC_TOBIAS && n != NPC_SERAPHINE &&
                n != NPC_BATMAN && n != NPC_GALAHAD && npcAvailable(n) && questReadyFor(n))
                tavern_flag = 1;

        printf("\n=================================\n");
        printf("            ELDERBROOK\n");
        printf("=================================\n");
        printf("Gold: %d   Level: %d (%s)   Locations cleared: %d/%d   Achievements: %d/%d\n",
               player.gold, player.level, rankName(player.level),
               progress.dungeonsCleared, DUNGEON_COUNT, achievementCount(), ACH_COUNT);
        printf("\n1. The Blacksmith%s\n", flag(NPC_BROKK));
        printf("2. The Tavern%s\n", tavern_flag ? " (!)" : "");
        printf("3. The Merchant\n");
        printf("4. The Training Area%s\n", training_flag ? " (!)" : "");
        printf("5. World Map (travel)\n");
        printf("6. Your pack\n");
        printf("7. Save and quit\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1: blacksmith(); saveNow(); break;
        case 2: tavern();     saveNow(); break;
        case 3: merchant();   saveNow(); break;
        case 4: training();   saveNow(); break;
        case 5:
        {
            int destination = worldMap();

            if (destination != 0)
                return destination;

            break;
        }
        case 6: pack(); break;
        case 7:
            saveNow();
            return HUB_QUIT;
        default:
            printf("Invalid choice.\n");
        }
    }
}
