#ifndef ENDINGS_H
#define ENDINGS_H

/* How much of the world you explored and the people you befriended.
   It decides which ending you get. */
int endingScore(void);

/* The bittersweet epilogue, now faction-aware. ids[] / count are the
   guilds that marched and fell at the Citadel (0 = you stood alone). */
void printEpilogue(const int *ids, int count);

/* The adventure summary shown at the very end. */
void printSummary(void);

#endif
