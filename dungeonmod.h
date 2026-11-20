#ifndef DUNGEONMOD_H
#define DUNGEONMOD_H

/* =========================================================
   DUNGEONMOD.H  —  Phase 5b: per-run dungeon modifiers (affixes)
   ---------------------------------------------------------
   When you enter a location, an optional modifier may be rolled
   that reshapes THAT run only: more fights, more hazards, a
   guaranteed elite, a calmer breather, or a guild turf war.

   Modifiers are TRANSIENT by design (locked decision Q4): they
   are never written to the save file. They bias the existing
   plan generator rather than replacing it, so old saved plans
   stay valid and the save format does not change.
   ========================================================= */

typedef enum
{
    DMOD_NONE = 0,
    DMOD_HAUNTED,       /* more hazards, richer relic-material drops   */
    DMOD_FLOODED,       /* more hazards, a watery, treacherous layout  */
    DMOD_CONTESTED,     /* a Hostile guild has moved in (Phase 3 glue) */
    DMOD_INFESTED,      /* a longer run, more fights, a guaranteed elite */
    DMOD_CALM,          /* a shorter, safer breather run               */
    DMOD_COUNT
} DungeonMod;

/* Roll a modifier for a run at this location (1..15). Stores it as the
   current modifier and returns it. DMOD_CONTESTED is only possible when
   you have driven at least one guild Hostile. */
DungeonMod rollDungeonMod(int level);

/* Force the current modifier (used to clear it to DMOD_NONE on revisits
   and when continuing a mid-run save, since modifiers are transient). */
void       setDungeonMod(DungeonMod m);
DungeonMod currentDungeonMod(void);

const char *dungeonModName(DungeonMod m);

/* Narrate the current modifier as you enter (prints nothing for NONE). */
void announceDungeonMod(void);

/* --- Planner knobs (read the current modifier) --- */
int modMonsterDelta(void);   /* change to the number of normal fights  */
int modExtraDelta(void);     /* change to the number of non-combat rooms */
int modForcesElite(void);    /* 1 = guarantee one elite room this run   */
int modHazardBias(void);     /* extra hazard rooms to weave in          */

/* Guild-Contested payoff: clearing the run nudges standing with the
   guild that moved in. Called once on a successful (non-revisit) clear. */
void dungeonModOnClear(int level);

#endif
