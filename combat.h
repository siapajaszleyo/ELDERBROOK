#ifndef COMBAT_H
#define COMBAT_H

#include "player.h"

/* Special tricks some enemies have */
typedef enum
{
    SPECIAL_NONE = 0,
    SPECIAL_REGEN,          /* heals a little every turn        */
    SPECIAL_DOUBLE,         /* sometimes strikes twice          */
    SPECIAL_RAGE,           /* hits harder when badly hurt      */
    SPECIAL_DRAIN,          /* drains the hero's mana           */
    SPECIAL_BURN            /* sometimes breathes fire (x2)     */
} EnemySpecial;

typedef enum { ENEMY_BRUTE, ENEMY_CASTER, ENEMY_SKIRMISHER,
               ENEMY_HEALER, ENEMY_TANK } EnemyArchetype;

typedef struct
{
    char name[50];

    int level;              /* enemies level up too; rank comes from it */

    int health;
    int maxHealth;

    int attack;
    int defense;

    int xpReward;
    int goldReward;

    EnemySpecial special;
    EnemySpecial special2;  /* some enemies have two tricks      */

    int type;               /* monster kind (-1 for bosses)      */
    int dropMaterial;       /* crafting material it may drop, -1 = none */
    int dropChance;         /* percent                           */
    EnemyArchetype archetype;
    int phase;

} Enemy;


void initializeEnemy(Enemy *enemy,
                     const char *name,
                     int level,
                     int health,
                     int attack,
                     int defense,
                     int xpReward,
                     int goldReward,
                     EnemySpecial special);

/* Run a whole fight.
   canRun: 1 = the hero may try to flee, 0 = no escape.
   Returns 1 = hero won, 0 = hero escaped, -1 = hero was defeated. */
int startCombat(Player *player, Enemy *enemy, int canRun);

void playerAttack(Player *player, Enemy *enemy);
void enemyAttack(Player *player, Enemy *enemy);

int isEnemyDefeated(const Enemy *enemy);
int isPlayerDefeated(const Player *player);

/* Player progression */
void gainXP(Player *player, int amount);
void levelUp(Player *player);

#endif
