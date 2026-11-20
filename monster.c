#include <stdlib.h>

#include "monster.h"
#include "world.h"
#include "game.h"
#include "balance.h"

/* One row per kind of monster. Stats are for level 1 and grow with level. */
typedef struct
{
    const char  *name;
    int          hp;
    int          atk;
    int          def;
    EnemySpecial special;
    EnemySpecial special2;
    int          dropMaterial;
    int          dropChance;        /* percent */
} MonsterType;

static const MonsterType monsterTypes[MT_COUNT] =
{
    /* name            hp  atk def  special          special2        drop         % */
    { "Slime",         45,  7,  2,  SPECIAL_REGEN,   SPECIAL_NONE,   MAT_HERB,    40 },
    { "Goblin",        35,  9,  3,  SPECIAL_DOUBLE,  SPECIAL_NONE,   MAT_IRON,    30 },
    { "Skeleton",      50, 10,  8,  SPECIAL_NONE,    SPECIAL_NONE,   MAT_IRON,    35 },
    { "Orc",           65, 13,  4,  SPECIAL_RAGE,    SPECIAL_NONE,   MAT_IRON,    35 },
    { "Wraith",        45, 12,  3,  SPECIAL_DRAIN,   SPECIAL_NONE,   MAT_SHADOW,  30 },
    { "Drake",         80, 16,  6,  SPECIAL_BURN,    SPECIAL_NONE,   MAT_SCALE,   25 },
    { "Cave Serpent",  55, 11,  4,  SPECIAL_DOUBLE,  SPECIAL_NONE,   MAT_HERB,    35 },
    { "Pirate",        60, 12,  4,  SPECIAL_DOUBLE,  SPECIAL_RAGE,   MAT_WOOD,    40 },
    { "Frost Wolf",    50, 12,  3,  SPECIAL_DOUBLE,  SPECIAL_NONE,   MAT_CRYSTAL, 25 },
    { "Ice Golem",     90, 11, 10,  SPECIAL_REGEN,   SPECIAL_NONE,   MAT_CRYSTAL, 40 },
    { "Fire Imp",      40, 13,  3,  SPECIAL_BURN,    SPECIAL_NONE,   MAT_CRYSTAL, 20 },
    { "Shadow Stalker",55, 15,  4,  SPECIAL_DRAIN,   SPECIAL_DOUBLE, MAT_SHADOW,  40 },
    { "Bone Knight",   85, 14, 10,  SPECIAL_RAGE,    SPECIAL_NONE,   MAT_RUNE,    15 },
    { "Wyvern",        90, 17,  6,  SPECIAL_BURN,    SPECIAL_DOUBLE, MAT_SCALE,   30 },
    { "Elemental",     95, 16,  6,  SPECIAL_BURN,    SPECIAL_REGEN,  MAT_CRYSTAL, 35 }
};


const char *monsterKindName(int kind)
{
    if (kind < 0 || kind >= MT_COUNT)
        return "Monster";

    return monsterTypes[kind].name;
}

void spawnMonsterOfKind(Enemy *enemy, int kind, int level)
{
    const MonsterType *type = &monsterTypes[kind];

    /* Scaling is driven entirely by balance.h so the game can be
       re-tuned without touching this logic. */
    int health  = type->hp * ENEMY_HP_BASE_MULT / 100
                  + (level - 1) * ENEMY_HP_PER_LEVEL;
    int attack  = type->atk + (level - 1) * ENEMY_ATK_PER_LEVEL;
    int defense = type->def + (level - 1) * ENEMY_DEF_PER_LEVEL;

    initializeEnemy(enemy, type->name, level,
                    health, attack, defense,
                    ENEMY_XP_BASE   + level * ENEMY_XP_PER_LEVEL,   /* xp reward   */
                    ENEMY_GOLD_BASE + level * ENEMY_GOLD_PER_LEVEL, /* gold reward */
                    type->special);

    enemy->special2     = type->special2;
    enemy->type         = kind;
    enemy->dropMaterial = type->dropMaterial;
    enemy->dropChance   = type->dropChance;
    if (kind == MT_SLIME)
        enemy->archetype = ENEMY_HEALER;
    else if (kind == MT_WRAITH || kind == MT_IMP || kind == MT_ELEMENTAL)
        enemy->archetype = ENEMY_CASTER;
    else if (kind == MT_GOBLIN || kind == MT_SERPENT || kind == MT_PIRATE ||
             kind == MT_FROSTWOLF || kind == MT_SHADOW)
        enemy->archetype = ENEMY_SKIRMISHER;
    else if (kind == MT_SKELETON || kind == MT_ICEGOLEM || kind == MT_BONEKNIGHT)
        enemy->archetype = ENEMY_TANK;
    else
        enemy->archetype = ENEMY_BRUTE;
}

void spawnMonster(Enemy *enemy, int dungeonLevel)
{
    const Location *where = getLocation(dungeonLevel);
    int kind = where->pool[rand() % where->poolCount];

    /* Monsters rank up with the location, and now and then one is a
       level stronger than usual. */
    int level = dungeonLevel + ((rand() % 100 < 25) ? 1 : 0);

    spawnMonsterOfKind(enemy, kind, level);
}

void spawnMiniboss(Enemy *enemy, int dungeonLevel, const char *name)
{
    initializeEnemy(enemy, name, dungeonLevel + MINIBOSS_LEVEL_BONUS,
                    MINIBOSS_HP_BASE  + dungeonLevel * MINIBOSS_HP_PER_LEVEL,  /* health */
                    MINIBOSS_ATK_BASE + dungeonLevel * MINIBOSS_ATK_PER_LEVEL, /* attack */
                    MINIBOSS_DEF_BASE + dungeonLevel * MINIBOSS_DEF_PER_LEVEL, /* defense */
                    40 + dungeonLevel * 10,
                    60 + dungeonLevel * 20,
                    SPECIAL_RAGE);
}
