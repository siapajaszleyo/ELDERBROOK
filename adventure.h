#ifndef ADVENTURE_H
#define ADVENTURE_H

#include "game.h"
#include "player.h"
#include "combat.h"
#include "guild.h"

/* Counters tracked for quests and achievements (indexes into adv.stats) */
enum
{
    ST_KILLS, ST_CHESS_WINS, ST_SNAKE_WINS, ST_DICE_WINS, ST_BJ_WINS,
    ST_GAMBLE_WINS, ST_FISH, ST_RARE_FISH, ST_CHESTS, ST_MEMORY_WINS,
    ST_REFLEX_WINS, ST_ARCHERY_BEST, ST_LOCKPICK_WINS, ST_CRAFTED,
    ST_SECRETS, ST_MINIBOSS_WINS, ST_BIG_HIT, ST_BJ_TOURNEY,
    ST_LEGEND_FISH, ST_TREASURES, ST_SHRINES, ST_NPC_TALKS, ST_TRAPS
};

/* Story switches (indexes into adv.flags) */
enum
{
    FLAG_GALAHAD_FOUND, FLAG_HAMMER_BACK, FLAG_WANDERER_TOKEN,
    FLAG_RECIPE_FROST, FLAG_RECIPE_FLAME, FLAG_RECIPE_DRAGON,
    FLAG_RECIPE_LEGEND, FLAG_RECIPE_SHADOW, FLAG_GAMBLER_TITLE,
    FLAG_INTRO_DONE, FLAG_ARTIFACT, FLAG_ARMOR_UPGRADES
};

/* Everything the Expanded Edition remembers, apart from the hero and
   the expedition (Player and Progress). It is saved with them. */
typedef struct
{
    int weapon;                       /* equipped item id, 0 = none     */
    int armor;
    int owned[ITEM_COUNT];            /* equipment the hero owns        */
    int materials[MAT_COUNT];
    int questState[QUEST_COUNT];      /* 0 not started, 1 active, 2 done */
    int questProgress[QUEST_COUNT];
    int npcMet[NPC_COUNT];
    int affinity[NPC_COUNT];          /* how much each character likes you */
    int achieved[ACH_COUNT];
    int secrets[SECRET_COUNT];
    int optBoss[OPTBOSS_COUNT];       /* optional bosses defeated       */
    int visited[DUNGEON_COUNT];
    int training[3];                  /* training challenges passed     */
    int flags[FLAG_COUNT];
    int stats[STAT_COUNT];
    int rodLevel;                     /* fishing rod quality            */

    /* ---- Character 2.0 fields ---- */
    int playerClass;                  /* CLASS_NONE..CLASS_GAMBLER     */
    int playerDiscipline;             /* DISC_NONE..DISC_DEALER         */
    int classAbilities[AB_COUNT_MAX]; /* 1 = learned                    */
    int relic;                        /* RELIC_NONE..RELIC_DRAGON_TOOTH */

    /* ---- Phase 4: nested forge (enchanting) ---- */
    int weaponEnchant;                /* 0 = none, else ENC_* id         */
    int armorEnchant;                 /* 0 = none, else ENC_* id         */

    /* ---- Phase 3: guilds & factions ---- */
    int guildRank[GUILD_COUNT];       /* 0 = not a member, 1..6 = rank  */
    int guildCreds[GUILD_COUNT];      /* per-guild bounty currency      */
    int guildStanding[GUILD_COUNT];   /* -100..+100 reputation          */
    int guildQuestStage[GUILD_COUNT]; /* 0..3 signature questline step  */

    /* Combat duration buffs/debuffs (set in combat, read here) */
    int ironFortressTurns;
    int warcryTurns;
    int burnTurns, burnDamage;
    int bleedTurns, bleedDamage;
    int poisonTurns, poisonDamage;
    int stormTurns, stormDamage;
    int markTurns;
    int huntersFocusTurns;
    int snareTurns;
    int grandTrapStun;
    int shadowStepTurns;
    int bloodPactTurns;
    int forbiddenPowerTurns;
    int pactSummonTurns, pactSummonDamage;
    int highRollerTurns;
    int stackedDeckTurns;

    /* One-shot combat flags */
    int precisionActive;
    int commandActive;
    int warlordsMightActive;
    int sacrificeActive;
    int disorientActive;
    int wildCardBuff;
    int cardSharpBuff;
    int doubleDownActive;
    int instantKill;
    int cryoFreeze;
    int cryoFreeze2;

    /* Run-scoped expedition modifiers and bonded knight openers. */
    int gateAttackDelta, gateDefenseDelta, gateHealthDelta, gateManaDelta;
    int gateCritDelta, gateInitiativeDelta, gateSpellDelta, gateChoice;
    int difficulty;                   /* 0 story, 1 standard, 2 veteran */
    int endlessDepth;                 /* deepest cleared endless floor */
    int vowReady, loanReady, darkReady;
    int vowUsed, loanUsed, darkUsed;
    int loanMana, loanShield;

} Adventure;

/* THE game state. Defined in adventure.c. */
extern Player    player;
extern Progress  progress;
extern Adventure adv;

void initializeAdventure(void);

const char *materialName(int material);
void grantMaterial(int material, int quantity);

/* Called after every victory. Handles drops, counters, quests. */
void onEnemyDefeated(const Enemy *enemy);
void grantOffense(Player *target, int amount);

#endif
