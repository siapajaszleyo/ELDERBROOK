#ifndef GAME_H
#define GAME_H

/* Shared settings and types used by several files. */

#define DUNGEON_COUNT   15    /* major locations on the adventure      */
#define NUM_KEYS        3     /* Fire, Earth, Air                      */
#define MAX_PLAN        16    /* most events one expedition can hold   */
#define MAX_SWORD_UPGRADES 3  /* sword upgrades the merchants will sell */

/* What can happen on an expedition.
   NEW EVENTS ARE ONLY EVER ADDED AT THE END, so plans stored in older
   save files keep their meaning. */
typedef enum
{
    EV_MONSTER = 0,           /* a regular fight                       */
    EV_ITEM,                  /* find a potion or gold                 */
    EV_MERCHANT,              /* wandering shop                        */
    EV_FORAGE,                /* herbs, wood, a little healing         */
    EV_DICE_TABLE,            /* optional gambling table               */
    EV_MINIBOSS,              /* the location's minigame boss (last)   */
    /* ---- added in the Expanded Edition ---- */
    EV_STORY,                 /* a short story scene with a choice     */
    EV_FISHING,               /* a fishing spot                        */
    EV_MINE,                  /* a vein of ore and crystal             */
    EV_CHEST,                 /* a locked chest (lockpicking)          */
    EV_ARCHERY,               /* an archery range                      */
    EV_BLACKJACK,             /* a blackjack table                     */
    EV_NPC,                   /* a recurring character appears         */
    EV_SECRET,                /* a secret room                         */
    EV_OPTBOSS,               /* an optional boss                      */
    /* ---- Phase 5b: dungeon variety ---- */
    EV_ELITE,                 /* a single, tougher "elite" fight       */
    EV_HAZARD,                /* an environmental room, no combat      */
    EV_SHRINE,                /* a rest / boon node                    */
    EV_BRANCH,                /* a fork: the long way or the quiet way */
    EV_COUNT
} EventType;

/* The minigames a miniboss can challenge you with */
typedef enum
{
    MG_CHESS = 0, MG_SNAKE, MG_DICE,
    MG_BLACKJACK, MG_MEMORY, MG_REFLEX, MG_ARCHERY, MG_LOCKPICK,
    MG_COUNT
} MinigameKind;

/* Results returned by the minigames (chess uses the same numbers) */
#define MINI_LOSE  0
#define MINI_WIN   1
#define MINI_QUIT  3

/* Crafting materials */
typedef enum
{
    MAT_IRON = 0, MAT_WOOD, MAT_HERB, MAT_CRYSTAL,
    MAT_SCALE, MAT_SHADOW, MAT_RUNE,
    MAT_COUNT
} Material;

/* Sizes of the tables used by the expansion (each table lives in its own
   module; these are here so the save system can see them). */
#define ITEM_COUNT      28    /* equipment (expanded: +bow, +staff)     */
#define SKILL_COUNT     9     /* 3 paths x 3 skills                     */
#define AB_COUNT_MAX    57    /* max class abilities (19 disciplines x 3) */
#define QUEST_COUNT     30
#define NPC_COUNT       12
#define ACH_COUNT       20
#define SECRET_COUNT    10
#define OPTBOSS_COUNT   5
#define FLAG_COUNT      24
#define STAT_COUNT      24
#define RELIC_COUNT_SAV 7     /* saveable relics                        */

#define ITEM_RUSTY_SWORD 1    /* the starting weapon                    */

/* Where the player is in the adventure. Saved with the Player. */
typedef struct
{
    int dungeonsCleared;      /* 0..DUNGEON_COUNT                      */
    int currentDungeon;       /* 0 = in town, else 1..15               */
    int planCount;            /* events planned for this expedition    */
    int planIndex;            /* next event to run                     */
    int plan[MAX_PLAN];       /* the events, in order                  */
    int revisit;              /* 1 = returning to a cleared location   */
} Progress;

#endif
