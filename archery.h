#ifndef ARCHERY_H
#define ARCHERY_H

/* Five arrows at a 5x3 target with a changing wind. Returns the total
   score (0-50). */
int playArchery(int level);

/* A miniboss contest: reach a target score. Returns MINI_WIN/MINI_LOSE. */
int archeryDuel(int level, const char *bossName);

#endif
