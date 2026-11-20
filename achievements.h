#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

/* Look at everything the hero has done and unlock whatever is earned.
   A banner is shown for each new achievement. Call it after events. */
void achievementCheck(void);

/* The achievement screen (shown in town). */
void showAchievements(void);

/* How many are unlocked */
int achievementCount(void);

#endif
