#include <stdio.h>

#include "achievements.h"
#include "adventure.h"
#include "quest.h"

enum
{
    ACH_FIRST_BLOOD, ACH_TREASURE_HUNTER, ACH_MASTER_GAMBLER, ACH_SNAKE_CHARMER,
    ACH_GRANDMASTER, ACH_OVERKILL, ACH_DRAGON_SLAYER, ACH_COMPLETIONIST,
    ACH_ANGLER, ACH_GOLDEN_KOI, ACH_LOCKMASTER, ACH_SHARPSHOOTER,
    ACH_SHARP_MEMORY, ACH_QUICK_REFLEXES, ACH_CRAFTSMAN, ACH_EXPLORER,
    ACH_KNIGHTS_FRIEND, ACH_SECRET_SEEKER, ACH_RICH, ACH_CARD_SHARK
};

typedef struct
{
    const char *name;
    const char *how;
} Achievement;

static const Achievement table[ACH_COUNT] =
{
    { "First Blood",       "Defeat your first monster." },
    { "Treasure Hunter",   "Open or find 8 treasures." },
    { "Master Gambler",    "Win 10 rounds of gambling." },
    { "Snake Charmer",     "Win a snake duel against a miniboss." },
    { "Grandmaster",       "Beat a miniboss at chess." },
    { "Overkill",          "Defeat a healthy monster with a single blow." },
    { "Dragon Slayer",     "Defeat the Ancient Dragon." },
    { "Completionist",     "Finish every quest, secret and optional boss." },
    { "Angler",            "Catch a rare fish." },
    { "Legend of the Lake","Catch the Golden Koi." },
    { "Lockmaster",        "Pick 3 locks." },
    { "Sharpshooter",      "Score 40 or more at archery." },
    { "Sharp Memory",      "Win a memory trial." },
    { "Quick Reflexes",    "Win a reflex challenge." },
    { "Craftsman",         "Craft something at the forge." },
    { "Explorer",          "Visit all 15 locations." },
    { "Friend of Knights", "Befriend all three knights." },
    { "Secret Seeker",     "Discover 3 secret rooms." },
    { "Well Off",          "Hold 1,000 gold at once." },
    { "Card Shark",        "Win 5 hands of blackjack." }
};

static int earned(int id)
{
    int i;

    switch (id)
    {
    case ACH_FIRST_BLOOD:     return adv.stats[ST_KILLS] >= 1;
    case ACH_TREASURE_HUNTER: return adv.stats[ST_TREASURES] >= 8;
    case ACH_MASTER_GAMBLER:  return adv.stats[ST_GAMBLE_WINS] >= 10;
    case ACH_SNAKE_CHARMER:   return adv.stats[ST_SNAKE_WINS] >= 1;
    case ACH_GRANDMASTER:     return adv.stats[ST_CHESS_WINS] >= 1;
    case ACH_OVERKILL:        return adv.stats[ST_BIG_HIT] >= 1;
    case ACH_DRAGON_SLAYER:   return adv.optBoss[0];
    case ACH_COMPLETIONIST:
        for (i = 0; i < QUEST_COUNT; i++)   if (adv.questState[i] != 2) return 0;
        for (i = 0; i < SECRET_COUNT; i++)  if (!adv.secrets[i])        return 0;
        for (i = 0; i < OPTBOSS_COUNT; i++) if (!adv.optBoss[i])        return 0;
        return 1;
    case ACH_ANGLER:          return adv.stats[ST_RARE_FISH] >= 1;
    case ACH_GOLDEN_KOI:      return adv.stats[ST_LEGEND_FISH] >= 1;
    case ACH_LOCKMASTER:      return adv.stats[ST_LOCKPICK_WINS] >= 3;
    case ACH_SHARPSHOOTER:    return adv.stats[ST_ARCHERY_BEST] >= 40;
    case ACH_SHARP_MEMORY:    return adv.stats[ST_MEMORY_WINS] >= 1;
    case ACH_QUICK_REFLEXES:  return adv.stats[ST_REFLEX_WINS] >= 1;
    case ACH_CRAFTSMAN:       return adv.stats[ST_CRAFTED] >= 1;
    case ACH_EXPLORER:
        for (i = 0; i < DUNGEON_COUNT; i++) if (!adv.visited[i]) return 0;
        return 1;
    case ACH_KNIGHTS_FRIEND:  return questKnightsFriendly();
    case ACH_SECRET_SEEKER:   return adv.stats[ST_SECRETS] >= 3;
    case ACH_RICH:            return player.gold >= 1000;
    case ACH_CARD_SHARK:      return adv.stats[ST_BJ_WINS] >= 5;
    }

    return 0;
}

void achievementCheck(void)
{
    for (int id = 0; id < ACH_COUNT; id++)
    {
        if (adv.achieved[id] || !earned(id))
            continue;

        adv.achieved[id] = 1;

        printf("\n  +------------------------------------+\n");
        printf("  |      ACHIEVEMENT UNLOCKED!         |\n");
        printf("  |  %-34s|\n", table[id].name);
        printf("  +------------------------------------+\n");
    }
}

int achievementCount(void)
{
    int n = 0;

    for (int id = 0; id < ACH_COUNT; id++)
        n += adv.achieved[id];

    return n;
}

void showAchievements(void)
{
    printf("\n=================================\n");
    printf("        ACHIEVEMENTS (%d/%d)\n", achievementCount(), ACH_COUNT);
    printf("=================================\n");

    for (int id = 0; id < ACH_COUNT; id++)
        printf("[%c] %-20s %s\n", adv.achieved[id] ? 'X' : ' ', table[id].name, table[id].how);
}
