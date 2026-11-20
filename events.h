#ifndef EVENTS_H
#define EVENTS_H

#include "game.h"

/* Which events exist in this build (a bit per EV_* value). The planner
   only chooses events that are ready. */
#define EVENTS_READY ((1 << EV_MONSTER) | (1 << EV_ITEM) | (1 << EV_MERCHANT) | \
                      (1 << EV_FORAGE) | (1 << EV_DICE_TABLE) | \
                      (1 << EV_MINIBOSS) | (1 << EV_STORY) | \
                      (1 << EV_FISHING) | (1 << EV_MINE) | (1 << EV_CHEST) | \
                      (1 << EV_ARCHERY) | (1 << EV_BLACKJACK) | \
                      (1 << EV_NPC) | (1 << EV_SECRET) | (1 << EV_OPTBOSS) | \
                      (1 << EV_ELITE) | (1 << EV_HAZARD) | \
                      (1 << EV_SHRINE) | (1 << EV_BRANCH))

int  eventIsReady(int event);
void runEvent(int event, int dungeonLevel);

void encounterMonster(int dungeonLevel);
void findItem(void);
void merchantEncounter(void);
void foragingOption(void);
void diceTable(int dungeonLevel);
void minibossEncounter(int dungeonLevel);
void storyVignette(int dungeonLevel);
void chestEvent(int dungeonLevel);
void mineEvent(int dungeonLevel);
void archeryEvent(int dungeonLevel);
void blackjackTable(int dungeonLevel);
void fishingEvent(int dungeonLevel);

/* ---- Phase 5b: dungeon variety room handlers ---- */
void eliteEncounter(int dungeonLevel);
void hazardRoom(int dungeonLevel);
void shrineRoom(int dungeonLevel);
void branchRoom(int dungeonLevel);

#endif
