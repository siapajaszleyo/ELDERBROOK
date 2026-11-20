#ifndef ENCOUNTERS_H
#define ENCOUNTERS_H

/* Chance (percent) of something unexpected between two events */
#define RANDOM_ENCOUNTER_PERCENT 25

/* An unexpected encounter on the road: a wandering merchant, a thief,
   treasure, an ambush, a lost traveller, a shrine, a trap, a mysterious
   chest or a travelling gambler. */
void randomEncounter(int dungeonLevel);

#endif
