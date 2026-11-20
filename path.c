#include <stdio.h>

#include "path.h"
#include "balance.h"
#include "class.h"
#include "adventure.h"

/* Class-flavoured titles, one per stage (Initiate, Adept, Master). */
static const char *titles[CLASS_COUNT][3] =
{
    /* CLASS_NONE    */ { "Wanderer",   "Wanderer",      "Wanderer"        },
    /* CLASS_KNIGHT  */ { "Squire",     "Knight-Errant", "Grand Champion"  },
    /* CLASS_ARCANIST*/ { "Apprentice", "Mage",          "Archmage"        },
    /* CLASS_ROGUE   */ { "Cutpurse",   "Rogue",         "Shadowblade"     },
    /* CLASS_RANGER  */ { "Tracker",    "Ranger",        "Warden"          },
    /* CLASS_WARLOCK */ { "Dabbler",    "Warlock",       "Dreadlord"       },
    /* CLASS_GAMBLER */ { "Chancer",    "Gambler",       "High Roller"     }
};

const char *pathStageName(int stage)
{
    switch (stage)
    {
    case PATH_INITIATE: return "Initiate";
    case PATH_ADEPT:    return "Adept";
    case PATH_MASTER:   return "Master";
    default:            return "Initiate";
    }
}

const char *pathTitle(const Player *player)
{
    int c = player->playerClass;
    int s = player->pathStage;
    if (c < 0 || c >= CLASS_COUNT) c = 0;
    if (s < 0 || s > 2) s = 0;
    return titles[c][s];
}

int pathStageForLevel(int level)
{
    if (level >= PATH_MASTER_LEVEL) return PATH_MASTER;
    if (level >= PATH_ADEPT_LEVEL)  return PATH_ADEPT;
    return PATH_INITIATE;
}

/* Unlock a discipline ability tier (1..3) for the player's discipline. */
static void unlockTier(const Player *player, int tier)
{
    int disc = player->playerDiscipline;
    if (disc <= DISC_NONE || disc >= DISC_COUNT)
        return;

    int ab = (disc - 1) * 3 + (tier - 1);
    if (ab < 0 || ab >= AB_COUNT)
        return;

    if (!adv.classAbilities[ab])
    {
        adv.classAbilities[ab] = 1;
        printf("Your mastery unlocks a new ability: %s!\n", abilityName(ab));
    }
}

static void promoteTo(Player *player, int stage)
{
    player->pathStage = stage;

    printf("\n*********************************************\n");
    printf("   PATH ADVANCED — you are now a %s %s!\n",
           pathStageName(stage), className(player->playerClass));
    printf("   Title: %s\n", pathTitle(player));
    printf("*********************************************\n");

    if (stage == PATH_ADEPT)
    {
        player->maxHealth += PATH_ADEPT_HP;  player->health += PATH_ADEPT_HP;
        player->maxMana   += PATH_ADEPT_MP;  player->mana   += PATH_ADEPT_MP;
        grantOffense(player, PATH_ADEPT_ATK);
        player->defense   += PATH_ADEPT_DEF;
        printf("You grow into your calling: +%d HP, +%d MP, +%d %s, +%d DEF.\n",
               PATH_ADEPT_HP, PATH_ADEPT_MP, PATH_ADEPT_ATK,
               (player->playerClass == CLASS_ARCANIST || player->playerClass == CLASS_WARLOCK) ? "MP" : "ATK",
               PATH_ADEPT_DEF);
        unlockTier(player, 2);   /* second discipline ability */
    }
    else if (stage == PATH_MASTER)
    {
        player->maxHealth += PATH_MASTER_HP;  player->health += PATH_MASTER_HP;
        player->maxMana   += PATH_MASTER_MP;  player->mana   += PATH_MASTER_MP;
        grantOffense(player, PATH_MASTER_ATK);
        player->defense   += PATH_MASTER_DEF;
        player->critChance += PATH_MASTER_CRIT;
        if (player->critChance > 100) player->critChance = 100;
        printf("You fully embrace the role: +%d HP, +%d MP, +%d %s, +%d DEF, +%d%% crit.\n",
               PATH_MASTER_HP, PATH_MASTER_MP, PATH_MASTER_ATK,
               (player->playerClass == CLASS_ARCANIST || player->playerClass == CLASS_WARLOCK) ? "MP" : "ATK",
               PATH_MASTER_DEF, PATH_MASTER_CRIT);
        unlockTier(player, 3);   /* capstone discipline ability */
    }
}

void pathCheckAdvance(Player *player)
{
    int target = pathStageForLevel(player->level);

    while (player->pathStage < target)
        promoteTo(player, player->pathStage + 1);
}

void pathShow(const Player *player)
{
    printf("\n--- Your Path ---\n");
    printf("%s  |  Stage: %s (%s)\n",
           className(player->playerClass),
           pathStageName(player->pathStage), pathTitle(player));
    printf("Discipline: %s\n", disciplineName(player->playerDiscipline));

    if (player->pathStage < PATH_ADEPT)
        printf("Next: reach level %d to become an Adept.\n", PATH_ADEPT_LEVEL);
    else if (player->pathStage < PATH_MASTER)
        printf("Next: reach level %d to become a Master.\n", PATH_MASTER_LEVEL);
    else
        printf("You have fully embraced your Path.\n");
}
