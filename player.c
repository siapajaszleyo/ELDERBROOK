#include <stdio.h>
#include <string.h>

#include "player.h"

void initializePlayer(Player *player, const char *name)
{
strncpy(player->name, name, sizeof(player->name) - 1);
player->name[sizeof(player->name) - 1] = '\0';

player->maxHealth = MAX_HEALTH;
player->health = MAX_HEALTH;

player->maxMana = MAX_MANA;
player->mana = MAX_MANA;

player->attack = 20;
player->defense = 10;

player->healPower = 20;

player->potions = 3;
player->manaPotions = 2;

player->gold = 100;

player->level = 1;
player->xp = 0;
player->xpToNextLevel = 35;

player->knightChosen = 0;
player->swordUpgrades = 0;

for (int i = 0; i < 3; i++)
    player->keys[i] = 0;

player->critChance = 5;
player->weaponFx = 0;
player->armorFx = 0;
player->skillPoints = 0;
for (int i = 0; i < SKILL_COUNT; i++)
    player->skills[i] = 0;
player->elixirs = 0;
player->fireBombs = 0;
player->smokeBombs = 0;

/* Character 2.0 defaults */
player->playerClass = 0;     /* CLASS_NONE */
player->playerDiscipline = 0; /* DISC_NONE */
player->relic = 0;          /* RELIC_NONE */
player->evasion = 0;
player->pathStage = 0;      /* Initiate */

}

void showPlayerStats(const Player *player)
{
printf("\n================================\n");
printf(" PLAYER STATS\n");
printf("================================\n");

printf("Name: %s\n", player->name);
printf("Level: %d (%s)\n", player->level, rankName(player->level));
printf("XP: %d/%d\n",
       player->xp,
       player->xpToNextLevel);

printf("\nHealth: %d/%d\n",
       player->health,
       player->maxHealth);

printf("Mana: %d/%d\n",
       player->mana,
       player->maxMana);

printf("Attack: %d\n", player->attack);
printf("Defense: %d\n", player->defense);

printf("\nGold: %d\n", player->gold);

printf("Health Potions: %d\n", player->potions);
printf("Mana Potions: %d\n", player->manaPotions);

printf("\nKeys:\n");
printf("Fire:  %s\n", player->keys[0] ? "[X]" : "[ ]");
printf("Earth: %s\n", player->keys[1] ? "[X]" : "[ ]");
printf("Air:   %s\n", player->keys[2] ? "[X]" : "[ ]");

printf("================================\n");


}

void healPlayer(Player *player, int amount)
{
player->health += amount;

if (player->health > player->maxHealth)
    player->health = player->maxHealth;


}

void restoreMana(Player *player, int amount)
{
player->mana += amount;

if (player->mana > player->maxMana)
    player->mana = player->maxMana;


}

void damagePlayer(Player *player, int amount)
{
player->health -= amount;

if (player->health < 0)
    player->health = 0;


}

const char *rankName(int level)
{
    if (level <= 2) return "Novice";
    if (level <= 4) return "Adept";
    if (level <= 6) return "Veteran";
    if (level <= 8) return "Elite";
    return "Legend";
}
