#ifndef ITEMS_H
#define ITEMS_H

#include "game.h"

#define SLOT_NONE    0
#define SLOT_WEAPON  1
#define SLOT_ARMOR   2

/* Special effects (stored in player.weaponFx / player.armorFx) */
#define WFX_FLAME    1     /* +6 damage                       */
#define WFX_FROST    2     /* 20% chance to freeze the enemy  */
#define WFX_DRAGON   3     /* +10 damage                      */
#define WFX_LEGEND   4     /* +8 damage, heal 3 per hit       */
#define WFX_PLUNDER  5     /* +4 damage, 5 gold per hit       */
#define WFX_DUEL     6     /* counter-attack on dodge         */
#define WFX_EXECUTE  7     /* bonus damage to low-HP enemies  */
#define WFX_SPELL    8     /* spell damage amplified          */
#define WFX_BLOOD    9     /* lifesteal                       */
#define WFX_GAMBLE   10    /* crit varies 5-35%               */

#define AFX_SHADOW   1     /* 15% chance to dodge             */
#define AFX_DRAGON   2     /* fire breath does less           */
#define AFX_AEGIS    4     /* every hit does 3 less           */
#define AFX_MAGUS    5     /* spell cost -15%                 */
#define AFX_BLOOD_A  6     /* +5 HP regen per turn            */
#define AFX_GAMBLE   7     /* gold bonus after combat         */
#define AFX_CLOAK    8     /* stealth: first strike bonus     */

typedef struct
{
    const char *name;
    int slot;
    int attack, defense, health, mana, crit;
    int fx;
    int mats[MAT_COUNT];        /* crafting cost                   */
    int gold;
    int minCleared;             /* locations cleared to see recipe */
    int reqFlag;                /* story flag needed, -1 = none    */
    int reqBoss;                /* optional boss needed, -1 = none */
    const char *description;
} Item;

const Item *getItem(int id);

/* Put an item on (replacing what is in that slot) */
void equipItem(int id);

/* Give an item; wear it at once if it is better than what you have. */
void grantItem(int id);

void showEquipment(void);
void equipmentMenu(void);
void craftingMenu(void);

#endif
