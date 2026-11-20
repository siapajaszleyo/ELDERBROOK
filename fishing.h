#ifndef FISHING_H
#define FISHING_H

/* Fish rarities */
enum { FISH_JUNK, FISH_COMMON, FISH_RARE, FISH_TREASURE, FISH_SPECIAL, FISH_LEGEND };

/* A fishing spot: several casts, each a little tug-of-war with whatever
   bites. Rewards go straight to the hero. */
void fishingSpot(int level);

/* Play out a single catch of a chosen rarity (used by tests). */
void fishingCatch(int level, int rarity);

#endif
