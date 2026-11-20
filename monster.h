#ifndef MONSTER_H
#define MONSTER_H

#include "combat.h"

/* Every kind of regular monster. Add a kind here, in the table in
   monster.c, and in a location's monster pool in world.c. */
typedef enum
{
    MT_SLIME = 0, MT_GOBLIN, MT_SKELETON, MT_ORC, MT_WRAITH, MT_DRAKE,
    MT_SERPENT, MT_PIRATE, MT_FROSTWOLF, MT_ICEGOLEM, MT_IMP, MT_SHADOW,
    MT_BONEKNIGHT, MT_WYVERN, MT_ELEMENTAL,
    MT_COUNT
} MonsterKind;

const char *monsterKindName(int kind);

/* Pick a random monster from this location's pool (location 1..15). */
void spawnMonster(Enemy *enemy, int dungeonLevel);

/* A specific kind at a specific level (used by quests and bosses). */
void spawnMonsterOfKind(Enemy *enemy, int kind, int level);

/* The sword-fight version of a location's miniboss. */
void spawnMiniboss(Enemy *enemy, int dungeonLevel, const char *name);

#endif
