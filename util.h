#ifndef UTIL_H
#define UTIL_H

/* Wait for a number of milliseconds. Skipped if the environment variable
   RPG_FAST is set (handy for testing). */
void pauseMs(int ms);

/* Push the screen up so earlier text scrolls out of sight. */
void scrollAway(void);

/* 1 when RPG_FAST is set */
int fastMode(void);

#endif
