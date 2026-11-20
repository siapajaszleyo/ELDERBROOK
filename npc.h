#ifndef NPC_H
#define NPC_H

/* The recurring cast. The number is the index in adv.npcMet/affinity. */
enum
{
    NPC_BROKK = 0,     /* blacksmith             */
    NPC_MARLA,         /* tavern keeper          */
    NPC_TOBIAS,        /* merchant               */
    NPC_ALDRIC,        /* old knight             */
    NPC_WANDERER,      /* mysterious traveller   */
    NPC_FINN,          /* gambler                */
    NPC_SERAPHINE,     /* mage                   */
    NPC_PETE,          /* fisherman              */
    NPC_ZARA,          /* treasure hunter        */
    NPC_GALAHAD,       /* the knights you may    */
    NPC_MORGANA,       /* free at the end        */
    NPC_BATMAN
};

const char *npcName(int npc);

/* Is this character around right now? (Some only arrive later.) */
int npcAvailable(int npc);

/* Talk, hand in quests, take new ones. Used in town and in dungeons. */
void talkTo(int npc);

/* A recurring character appears during an expedition. */
void npcEncounter(int dungeonLevel);

/* Affinity (0-10) with a knight, used by the ending */
int npcAffinity(int npc);

#endif
