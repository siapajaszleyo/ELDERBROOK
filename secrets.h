#ifndef SECRETS_H
#define SECRETS_H

/* Secret rooms (EV_SECRET) and optional bosses (EV_OPTBOSS). */

int  secretFound(int id);
const char *secretName(int id);

/* Is the way to this optional boss open yet? */
int  optBossUnlocked(int id);

void secretRoom(int dungeonLevel);
void optionalBoss(int dungeonLevel);

int  secretsFoundCount(void);
int  optBossesDefeatedCount(void);

#endif
