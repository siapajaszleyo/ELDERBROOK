#ifndef WORLD_H
#define WORLD_H

#include "game.h"

/* One of the 15 major locations. Each has its own identity: monsters,
   the mix of events, a miniboss, and its own story scene. */
typedef struct
{
    const char *name;
    const char *arrival;            /* shown when you arrive            */
    const char *monsterIntro;       /* printf format, one %s = monster  */

    int pool[4];                    /* monster kinds found here         */
    int poolCount;

    int extras[8];                  /* non-combat events (repeat = more likely) */
    int extraCount;

    int minibossKind;               /* MG_* minigame                    */
    const char *minibossName;
    const char *minibossIntro;

    const char *story;              /* a story scene with a choice      */
    const char *storyA;             /* the careful option               */
    const char *storyB;             /* the risky option                 */

    int secret;                     /* secret room id, or -1            */
    int optBoss;                    /* optional boss id, or -1          */
} Location;

/* level is 1..DUNGEON_COUNT */
const Location *getLocation(int level);

/* Plan the events of an expedition into progress.plan.
   revisit = 1 skips the story scene and the miniboss. */
void buildPlanFor(Progress *progress, int level, int revisit);

#endif
