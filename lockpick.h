#ifndef LOCKPICK_H
#define LOCKPICK_H

/* Crack a combination lock using clues. Returns MINI_WIN, MINI_LOSE
   (out of attempts) or MINI_QUIT. */
int playLockpick(int level, const char *lockName);

#endif
