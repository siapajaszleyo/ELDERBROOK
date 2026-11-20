#ifndef QUEST_H
#define QUEST_H

#include "game.h"

enum
{
    Q_KNIGHT_RESCUE = 0, Q_HAMMER, Q_TREASURE_MAP, Q_SHADOW_BEAST,
    Q_FISHERMAN, Q_ARTIFACT, Q_TOURNAMENT, Q_RARE_MATERIALS,
    Q_HERBS, Q_BRAVERY, Q_OATH, Q_WANDERER,
    Q_VOW_PATROL, Q_VOW_IRON, Q_VOW_UNDEAD,
    Q_ARCANUM_RUNES, Q_ARCANUM_LOCKS, Q_ARCANUM_LIBRARY,
    Q_HAND_PIRATES, Q_HAND_TOURNEY, Q_HAND_SHADOW,
    Q_WARDENS_FISH, Q_WARDENS_WOLVES, Q_WARDENS_SKY,
    Q_COVENANT_ESSENCE, Q_COVENANT_SHADOWS, Q_COVENANT_PRISM,
    Q_WHEEL_ARCHERY, Q_WHEEL_CRYSTALS, Q_WHEEL_CITADEL
};

/* Things the quest system wants to hear about */
enum { QN_KILL, QN_FISH_RARE, QN_CHEST, QN_BJ_BIG_WIN };

void questNotify(int kind, int param);

/* Offer this character's next quest, if any. Returns 1 if one was offered. */
int  questOfferFor(int npc);

/* Complete and reward any finished quests this character gave. Returns the number completed. */
int  questTurnInFor(int npc);

/* Is something waiting to be handed in to this character? */
int  questReadyFor(int npc);

void questJournal(void);

/* Used by the achievements */
int  questKnightsFriendly(void);
int  questsCompleted(void);

#endif
