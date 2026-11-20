#ifndef BALANCE_H
#define BALANCE_H

/* =========================================================
   BALANCE.H  —  Expanded Edition tuning knobs
   ---------------------------------------------------------
   ALL combat/economy scaling numbers live here so the game can be
   re-tuned by editing ONE file. Nothing in here changes game logic;
   the .c files read these constants. This is the "edit the .h to
   rebalance" workflow the project is built around.
   ========================================================= */

/* ---- Enemy scaling (per level above 1) ----
   The old values (hp +8, atk +1.5, def +0.5) let the hero out-scale
   every enemy and one-shot them by mid game. These make enemies grow
   fast enough to stay a threat into the late game. */
#define ENEMY_HP_BASE_MULT      100   /* percent applied to a type's base hp  */
#define ENEMY_HP_PER_LEVEL       22   /* +hp per level above 1                */
#define ENEMY_ATK_PER_LEVEL       3   /* +attack per level above 1           */
#define ENEMY_DEF_PER_LEVEL       1   /* +defense per level above 1          */

/* ---- Miniboss scaling ---- */
#define MINIBOSS_HP_BASE         140
#define MINIBOSS_HP_PER_LEVEL     30
#define MINIBOSS_ATK_BASE         16
#define MINIBOSS_ATK_PER_LEVEL     2
#define MINIBOSS_DEF_BASE          8
#define MINIBOSS_DEF_PER_LEVEL     1
#define MINIBOSS_LEVEL_BONUS       2   /* minibosses are this many levels up  */

/* ---- Rewards scaling ---- */
#define ENEMY_XP_BASE             15
#define ENEMY_XP_PER_LEVEL         5
#define ENEMY_GOLD_BASE           15
#define ENEMY_GOLD_PER_LEVEL      10

/* ---- Economy: healing is no longer free ---- */
/* Returning to town gives only a small free rest; a full restore now
   costs gold at the tavern (a real gold sink). */
#define INN_FREE_REST_PERCENT     25   /* free HP/MP % restored on arriving   */
#define TAVERN_HEAL_HP_COST        2   /* gold per 1 HP healed at the tavern  */
#define TAVERN_HEAL_MP_COST        3   /* gold per 1 MP restored at the tavern*/
#define TAVERN_FULL_HEAL_DISCOUNT 10   /* % off when buying a full restore    */
#define TAVERN_DRINK_COST        120   /* a "drink": temporary combat buff    */

/* ---- Guild creds (bounty) economy ---- */
#define CREDS_PER_QUEST           20
#define CREDS_PER_OPTBOSS         60
#define CREDS_PER_SECRET          10
#define GUILD_CREDS_PER_KILL       2   /* trickle to your home guild per kill */

/* ---- Guild ranks (6 per guild, paced to the level-20 cap) ---- */
/* Creds dues to REACH each rank (index = rank 1..6; rank 1 is free join). */
#define GUILD_DUES_R2            100
#define GUILD_DUES_R3            220
#define GUILD_DUES_R4            380
#define GUILD_DUES_R5            600
#define GUILD_DUES_R6            900
/* Standing bumps */
#define STANDING_JOIN_SELF        50   /* home guild starts here              */
#define STANDING_PER_QUEST_TIER   18   /* questline step with your guild      */
#define STANDING_PER_OPTBOSS       8
#define STANDING_PER_SECRET        3
#define STANDING_MIN            -100
#define STANDING_MAX             100

/* ---- Faction standing tiers ---- */
#define GUILD_ALLIED_THRESHOLD    60
#define GUILD_FRIENDLY_THRESHOLD  20
#define GUILD_WARY_THRESHOLD     -20
#define GUILD_HOSTILE_THRESHOLD  -60

/* ---- Hostile ambushes (enabled) ---- */
#define AMBUSH_BASE_CHANCE        14   /* % per hostile guild, on map entry   */
#define AMBUSH_MAX_CHANCE         60   /* cap so travel is never impossible   */
#define AMBUSH_KILL_STANDING      -4   /* killing their enforcers angers them */

/* ---- Path system (multi-stage class journey) ---- */
#define LEVEL_CAP                 20   /* hard level ceiling                 */
#define PATH_ADEPT_LEVEL           6   /* reach Stage II (Adept) here        */
#define PATH_MASTER_LEVEL         13   /* reach Stage III (Master) here      */
/* Stat rewards granted the moment you advance a Path stage */
#define PATH_ADEPT_HP             15
#define PATH_ADEPT_MP             10
#define PATH_ADEPT_ATK             5
#define PATH_ADEPT_DEF             3
#define PATH_MASTER_HP            25
#define PATH_MASTER_MP            15
#define PATH_MASTER_ATK            8
#define PATH_MASTER_DEF            5
#define PATH_MASTER_CRIT           5

/* ---- Phase 4: nested forge (enchanting + relic crafting) ---- */
/* Covenant rank required to forge relics (4 = Pactbearer). */
#define COVENANT_RELIC_RANK        4
/* Covenant Creds charged on top of the material cost per relic. */
#define RELIC_CREDS_EYE          200
#define RELIC_CREDS_BLOODSTONE   200
#define RELIC_CREDS_COIN         250
#define RELIC_CREDS_FROZEN       220
#define RELIC_CREDS_COMPASS      220
#define RELIC_CREDS_DRAGON       300

#endif
