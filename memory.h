#ifndef MEMORY_H
#define MEMORY_H

/* A sequence of colours flashes up, then vanishes. Type it back.
   Three rounds, each longer than the last; pass two to win.
   Returns MINI_WIN or MINI_LOSE (see game.h). */
int playMemory(int level, const char *bossName);

#endif
