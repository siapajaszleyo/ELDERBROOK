#ifndef GUILD_H
#define GUILD_H

#include "class.h"

/* =========================================================
   GUILD.H  —  Six guilds, Creds, ranks & faction politics
   ---------------------------------------------------------
   Phase 3 of the Expanded Edition. Every class is born into one
   "home guild" tied to its Stage-I origin. The six guilds each live
   in their own building on the World Map, run their own 6-rank ladder
   paid for in per-guild Creds, and sit in a web of allied/rival
   standings. Helping one guild pleases its bloc and angers the other.
   Tuning knobs live in balance.h.
   ========================================================= */

typedef enum
{
    GUILD_IRON_VOW = 0,   /* Knight   — Castle of Ash      */
    GUILD_ARCANUM,        /* Arcanist — Ancient Library    */
    GUILD_SHROUDED_HAND,  /* Rogue    — Pirate's Den       */
    GUILD_WARDENS_LODGE,  /* Ranger   — Whispering Forest  */
    GUILD_COVENANT,       /* Warlock  — Shadow Realm       */
    GUILD_GILDED_WHEEL,   /* Gambler  — Sunken Temple      */
    GUILD_COUNT
} GuildId;

/* Standing tiers (derived from the -100..+100 standing value). */
typedef enum
{
    STAND_HOSTILE = 0,
    STAND_WARY,
    STAND_NEUTRAL,
    STAND_FRIENDLY,
    STAND_ALLIED
} StandingTier;

/* --- Identity --- */
const char *guildName(GuildId g);
const char *guildBlurb(GuildId g);
const char *guildRankName(GuildId g, int rank);   /* rank 0..6, 0 = non-member */
GuildId     homeGuildForClass(ClassKind c);        /* GUILD_COUNT if none       */

/* Which guild (if any) keeps its house at this World-Map location (1..15)?
   Returns GUILD_COUNT when the location is not a guild home. */
GuildId guildAtLocation(int level);
int     guildHomeLocation(GuildId g);              /* 1..15                     */

/* --- Politics --- */
/* +1 = allied, -1 = rival, 0 = neutral/flexible (Gilded Wheel pairs). */
int          guildAllyOf(GuildId g, GuildId other);
StandingTier guildTier(GuildId g);
StandingTier standingTierOf(int standing);
const char  *standingTierName(StandingTier t);

/* Change standing with guild g by n, applying the bloc spillover rule
   (allies +n/2, rivals -n/2, Arcanum<->Covenant full -n, Wheel ±n/4). */
void guildAddStanding(GuildId g, int n);

/* --- Creds --- */
void guildAddCreds(GuildId g, int amount);        /* clamps 0..999999          */
void guildEarnHomeCreds(int amount);              /* awards to your home guild */
/* Convenience hooks called from quest / secret / optboss resolution. */
void guildOnQuestComplete(void);
void guildOnOptBoss(void);
void guildOnSecret(void);

/* --- Seeding (called from class loadout) --- */
void guildSeedHome(ClassKind c);

/* --- UI --- */
void guildVisit(int level);     /* enter the guild house at this location */
void guildLedger(void);         /* the pack sub-screen listing all six    */

/* --- Hostile ambushes --- */
/* Called on entering a location. Rolls for an enforcer ambush from any
   guild you've driven Hostile. Returns 1 if the hero survived (or no
   ambush happened), 0 if the hero was defeated. */
int guildAmbushCheck(int level);

/* --- Finale support (Phase 5) --- */
/* Fills ids[] (size >= GUILD_COUNT) with the guilds that are Allied and
   will answer the call. Returns how many. */
int guildFinaleAllies(int *ids);

#endif
