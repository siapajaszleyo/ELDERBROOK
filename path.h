#ifndef PATH_H
#define PATH_H

#include "player.h"

/* =========================================================
   PATH.H  —  The multi-stage class journey (Expanded Edition)
   ---------------------------------------------------------
   Every hero walks a Path with three stages. Stage I (Initiate) is
   chosen at creation (your class = your origin). As you level you grow
   into the role: Stage II (Adept) and finally Stage III (Master), where
   you "fully embrace" who you are — unlocking your discipline's capstone
   ability and a surge of power. Stage thresholds and bonuses live in
   balance.h. The level ceiling is LEVEL_CAP.
   ========================================================= */

typedef enum
{
    PATH_INITIATE = 0,
    PATH_ADEPT    = 1,
    PATH_MASTER   = 2
} PathStage;

/* Plain stage name ("Initiate" / "Adept" / "Master"). */
const char *pathStageName(int stage);

/* The class-flavoured title for the hero's CURRENT stage
   (e.g. a Knight Master is a "Grand Champion"). */
const char *pathTitle(const Player *player);

/* Which stage does this character level qualify for? */
int pathStageForLevel(int level);

/* Promote the hero if their level has reached a new stage. Applies the
   stat rewards and unlocks the matching discipline ability tier.
   Safe to call every level-up; it only fires on an actual advance. */
void pathCheckAdvance(Player *player);

/* Print a short Path status panel. */
void pathShow(const Player *player);

#endif
