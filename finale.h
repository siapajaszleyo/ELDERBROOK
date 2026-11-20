#ifndef FINALE_H
#define FINALE_H

#include "combat.h"   /* Enemy */

/* =========================================================
   FINALE.H  —  Phase 5a: the faction muster
   ---------------------------------------------------------
   Guilds you raised to ALLIED standing may answer independently at the Final
   Citadel with you. Each lends one themed boon (to you) or one
   blow against the boss, then — one by one — they fall, buying
   the opening for the final strike, which is always yours alone.
   This is end-game and transient: it reads live standing and
   writes no new saved state.
   ========================================================= */

/* Gather the Allied guilds, narrate their arrival, and apply each
   guild's player-side boon right away. Fills ids[] (size >= GUILD_COUNT)
   with the guilds that marched and returns how many. */
int finaleMuster(int *ids);

/* Apply the boss-side contributions of the marched guilds (the Arcanum
   wards away the boss's deadliest trick; the Wardens stagger it). */
void finaleApplyToBoss(Enemy *boss, const int *ids, int count);

/* Narrate the allies falling one by one, then the hero's final blow.
   Used by both finale tracks (steel and chess). */
void finaleFalls(const int *ids, int count);

#endif
