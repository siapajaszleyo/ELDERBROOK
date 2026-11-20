#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

#define MAX_HEALTH 100
#define MAX_MANA 50

typedef struct
{
char name[50];

int health;
int maxHealth;

int mana;
int maxMana;

int attack;
int defense;

int healPower;

int potions;
int manaPotions;

int gold;

int level;
int xp;
int xpToNextLevel;

int knightChosen;

int swordUpgrades;

int keys[3];

/* ---- Expanded Edition ---- */
int critChance;             /* percent chance of a critical hit    */
int weaponFx;               /* special effect of the equipped weapon */
int armorFx;                /* special effect of the equipped armor  */
int skillPoints;
int skills[SKILL_COUNT];    /* 1 = learned                          */
int elixirs;                /* full heal, usable in a fight         */
int fireBombs;
int smokeBombs;

/* ---- Character 2.0 ---- */
int playerClass;            /* CLASS_NONE..CLASS_GAMBLER             */
int playerDiscipline;       /* DISC_NONE..DISC_DEALER                 */
int relic;                  /* RELIC_NONE..RELIC_DRAGON_TOOTH         */
int evasion;                /* evasion percent bonus                  */

int pathStage;              /* 0 Initiate, 1 Adept, 2 Master          */

} Player;

void initializePlayer(Player *player, const char *name);
void showPlayerStats(const Player *player);
void healPlayer(Player *player, int amount);
void restoreMana(Player *player, int amount);
void damagePlayer(Player *player, int amount);

/* Rank title for a level (used for the hero AND for monsters) */
const char *rankName(int level);

#endif