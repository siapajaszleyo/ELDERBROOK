#include <stdio.h>
#include <string.h>

#include "items.h"
#include "adventure.h"
#include "input.h"

/* THE EQUIPMENT TABLE. The index in this table is the item id that is
   saved in the save file, so NEVER reorder it; add new items at the end
   (up to ITEM_COUNT) and keep the ids of existing items. */
static const Item items[ITEM_COUNT] =
{
 /* 0 */ { "(nothing)",          SLOT_NONE,   0, 0, 0, 0, 0, 0, {0},                    0,   0, -1, -1, "" },
 /* 1 */ { "Rusty Sword",        SLOT_WEAPON, 0, 0, 0, 0, 0, 0, {0},                    0,   0, -1, -1, "Your trusty starting blade." },
 /* 2 */ { "Iron Sword",         SLOT_WEAPON, 8, 0, 0, 0, 0, 0, {4,2,0,0,0,0,0},       30,  1, -1, -1, "A sturdy, honest sword." },
 /* 3 */ { "Knight Sword",       SLOT_WEAPON,14, 0, 0, 0, 5, 0, {8,2,0,0,0,0,0},       80,  3, -1, -1, "Balanced steel with a keen edge." },
 /* 4 */ { "Flame Blade",        SLOT_WEAPON,18, 0, 0, 0, 0, WFX_FLAME,  {6,0,0,3,2,0,0}, 150, 8, -1, -1, "Burns what it cuts: +6 damage." },
 /* 5 */ { "Frost Blade",        SLOT_WEAPON,18, 0, 0, 0, 0, WFX_FROST,  {6,0,0,6,0,0,0}, 150, 7, -1, -1, "20% chance to freeze a foe in place." },
 /* 6 */ { "Dragon Blade",       SLOT_WEAPON,28, 0, 0, 0, 5, WFX_DRAGON, {8,0,0,0,6,0,2}, 300,12, -1, -1, "Forged from dragon scale: +10 damage." },
 /* 7 */ { "Legendary Sword",    SLOT_WEAPON,36, 0, 0, 0,10, WFX_LEGEND, {0,0,0,6,4,4,4}, 500,12, -1,  0, "Heals you a little with every hit." },
 /* 8 */ { "Leather Armor",      SLOT_ARMOR,  0, 4,10, 0, 0, 0, {1,3,0,0,0,0,0},       20,  1, -1, -1, "Light and quiet." },
 /* 9 */ { "Knight Armor",       SLOT_ARMOR,  0,10,20, 0, 0, 0, {10,3,0,0,0,0,0},      100, 4, -1, -1, "Heavy plate of the old order." },
 /*10 */ { "Frost Mail",         SLOT_ARMOR,  0,12,25, 0, 0, 0, {8,0,0,6,0,0,0},       120, 7, -1, -1, "Chilled mail that never rusts." },
 /*11 */ { "Shadow Armor",       SLOT_ARMOR,  0, 8,10,15, 0, AFX_SHADOW, {4,0,0,0,0,6,0}, 150, 9, -1, -1, "15% chance to slip away from a hit." },
 /*12 */ { "Dragon Armor",       SLOT_ARMOR,  0,16,40, 0, 0, AFX_DRAGON, {8,0,0,0,6,0,0}, 250,10, FLAG_RECIPE_DRAGON, -1, "Fire breath does far less to you." },
 /*13 */ { "Aegis of the Elements", SLOT_ARMOR, 0,20,30,20, 0, AFX_AEGIS, {0},          0,   0, -1, -1, "A reward. Every hit does 3 less." },
 /*14 */ { "Forgotten Blade",    SLOT_WEAPON,24, 0, 0, 0, 8, WFX_LEGEND, {0},           0,   0, -1, -1, "A reward. An old knight's sword." },
 /*15 */ { "Pirate King's Cutlass", SLOT_WEAPON,22,0,0,0,5, WFX_PLUNDER,{0},            0,   0, -1, -1, "A reward. Plunders gold with each hit." },
 /*16 */ { "Duelist Rapier",      SLOT_WEAPON,16, 0, 0, 0,10, WFX_DUEL,  {4,0,0,2,0,2,0},      160, 9, -1, -1, "Counter-attacks when you dodge." },
 /*17 */ { "Executioner's Axe",   SLOT_WEAPON,22, 0, 0, 0, 0, WFX_EXECUTE,{8,0,0,0,2,0,2},     200,11, -1, -1, "Massive bonus damage to low-HP foes." },
 /*18 */ { "Spellblade",          SLOT_WEAPON,14, 0, 0,15, 0, WFX_SPELL, {4,0,0,4,0,0,2},      180, 9, -1, -1, "Arcane edge: spell damage amplified." },
 /*19 */ { "Bloodfang",           SLOT_WEAPON,20, 0, 0, 0, 0, WFX_BLOOD, {6,0,0,0,0,4,2},      220,11, -1, -1, "Drains life from every wound it makes." },
 /*20 */ { "Gambler's Blade",     SLOT_WEAPON,15, 0, 0, 0,18, WFX_GAMBLE, {2,2,0,2,0,2,0},      160, 8, -1, -1, "Crit chance fluctuates: 5% to 35%." },
 /*21 */ { "Shadow Cloak",         SLOT_ARMOR,  0, 6,10, 0,10, AFX_CLOAK, {4,0,0,0,0,4,0},      140, 7, -1, -1, "Stealthy: first strike always crits." },
 /*22 */ { "Magus Robes",          SLOT_ARMOR,  0, 4,15,25, 0, AFX_MAGUS,  {4,0,0,4,0,0,2},      160, 8, -1, -1, "Spellcasting costs 15% less mana." },
 /*23 */ { "Bloodmail",            SLOT_ARMOR,  0,12,30, 0, 0, AFX_BLOOD_A,{6,0,0,0,0,4,2},      200,10, -1, -1, "Regenerates 5 HP per combat turn." },
 /*24 */ { "Gambler's Coat",       SLOT_ARMOR,  0, 4, 5, 0, 0, AFX_GAMBLE, {2,2,0,2,0,2,0},      120, 6, -1, -1, "Enemies drop 20% more gold." },
 /*25 */ { "Dragon Plate",         SLOT_ARMOR,  0,18,45, 0, 0, AFX_DRAGON, {10,0,0,0,6,0,2},     300,12, FLAG_RECIPE_DRAGON, -1, "Dragon-fire cannot pierce this plate." },
 /*26 */ { "Hunter's Longbow",     SLOT_WEAPON,16, 0, 0, 0, 8, 0, {4,6,0,0,0,0,0},      120, 2, -1, -1, "A ranged weapon: safe shots, aimed criticals." },
 /*27 */ { "Runed Staff",          SLOT_WEAPON,12, 0, 0,20, 0, WFX_SPELL, {4,4,0,4,0,0,0}, 140, 3, -1, -1, "A caster's staff: strikes feed on your mana." }
};

const Item *getItem(int id)
{
    if (id < 0 || id >= ITEM_COUNT)
        id = 0;

    return &items[id];
}

static int itemPower(int id)
{
    const Item *i = getItem(id);

    return i->attack * 2 + i->defense * 2 + i->health / 5 + i->mana / 5 + i->crit;
}

/* Add (sign = 1) or remove (sign = -1) an item's bonuses. */
static void applyItem(const Item *item, int sign)
{
    if (adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK)
        player.maxMana += sign * item->attack;
    else
        player.attack += sign * item->attack;
    player.defense    += sign * item->defense;
    player.maxHealth  += sign * item->health;
    player.maxMana    += sign * item->mana;
    player.critChance += sign * item->crit;
    if (player.critChance > 100) player.critChance = 100;
    if (player.critChance < 0) player.critChance = 0;

    if (player.health > player.maxHealth)
        player.health = player.maxHealth;
    if (player.mana > player.maxMana)
        player.mana = player.maxMana;
}

void equipItem(int id)
{
    const Item *item = getItem(id);

    if (item->slot == SLOT_NONE || !adv.owned[id])
        return;

    int *slot = (item->slot == SLOT_WEAPON) ? &adv.weapon : &adv.armor;

    if (*slot == id)
        return;

    applyItem(getItem(*slot), -1);         /* take the old one off */
    *slot = id;
    applyItem(item, +1);

    if (item->slot == SLOT_WEAPON)
        player.weaponFx = item->fx;
    else
        player.armorFx = item->fx;
}

void grantItem(int id)
{
    const Item *item = getItem(id);

    adv.owned[id] = 1;
    printf("\n*** You obtained: %s! ***\n", item->name);

    int *slot = (item->slot == SLOT_WEAPON) ? &adv.weapon : &adv.armor;

    if (item->slot != SLOT_NONE && itemPower(id) > itemPower(*slot))
    {
        equipItem(id);
        printf("It is better than what you wear, so you equip it.\n");
    }
}

static void describe(const Item *item)
{
    printf("%s", item->name);

    if (item->attack)  printf("  ATK +%d", item->attack);
    if (item->defense) printf("  DEF +%d", item->defense);
    if (item->health)  printf("  HP +%d", item->health);
    if (item->mana)    printf("  MP +%d", item->mana);
    if (item->crit)    printf("  CRIT +%d%%", item->crit);
}

void showEquipment(void)
{
    printf("\nWeapon: ");
    describe(getItem(adv.weapon));
    printf("\n        %s\n", getItem(adv.weapon)->description);

    printf("Armor:  ");
    if (adv.armor == 0)
        printf("(nothing)\n");
    else
    {
        describe(getItem(adv.armor));
        printf("\n        %s\n", getItem(adv.armor)->description);
    }
}

void equipmentMenu(void)
{
    while (1)
    {
        int list[ITEM_COUNT];
        int count = 0;

        printf("\n=================================\n");
        printf("          EQUIPMENT\n");
        printf("=================================\n");
        showEquipment();

        printf("\nIn your pack:\n");

        for (int id = 1; id < ITEM_COUNT; id++)
        {
            if (!adv.owned[id] || id == adv.weapon || id == adv.armor)
                continue;

            list[count++] = id;
            printf("%d. ", count);
            describe(getItem(id));
            printf("\n");
        }

        if (count == 0)
            printf("(nothing else yet: craft or find something!)\n");

        printf("\nEnter a number to equip it, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > count)
        {
            printf("Invalid choice.\n");
            continue;
        }

        equipItem(list[choice - 1]);
        printf("Equipped %s.\n", getItem(list[choice - 1])->name);
    }
}


/* =========================================================
   CRAFTING
   Equipment recipes come from the item table above. Potions and
   gadgets are listed here. Recipes appear as you clear locations.
   ========================================================= */

typedef struct
{
    const char *name;
    int mats[MAT_COUNT];
    int gold;
    int minCleared;
} Gadget;

enum { G_POTION, G_MANA, G_BOMB, G_SMOKE, G_ELIXIR, G_COUNT };

static const Gadget gadgets[G_COUNT] =
{
    { "Health Potion",   {0,0,3,0,0,0,0}, 0, 0 },
    { "Mana Potion",     {0,0,2,1,0,0,0}, 0, 0 },
    { "Fire Bomb",       {2,0,0,1,0,0,0}, 20, 3 },
    { "Smoke Bomb",      {0,2,0,0,0,1,0}, 20, 3 },
    { "Elixir (full heal)", {0,0,4,2,0,1,0}, 50, 5 }
};

static int canAfford(const int *mats, int gold)
{
    for (int m = 0; m < MAT_COUNT; m++)
        if (adv.materials[m] < mats[m])
            return 0;

    return player.gold >= gold;
}

static void pay(const int *mats, int gold)
{
    for (int m = 0; m < MAT_COUNT; m++)
        adv.materials[m] -= mats[m];

    player.gold -= gold;
}

static void printCost(const int *mats, int gold)
{
    int first = 1;

    for (int m = 0; m < MAT_COUNT; m++)
    {
        if (mats[m] <= 0)
            continue;

        printf("%s%d %s", first ? "" : ", ", mats[m], materialName(m));
        first = 0;
    }

    if (gold > 0)
        printf("%s%d gold", first ? "" : ", ", gold);
}

static int recipeVisible(const Item *item)
{
    if (item->gold == 0 && item->mats[0] == 0 && item->mats[1] == 0)
    {
        int any = 0;

        for (int m = 0; m < MAT_COUNT; m++)
            any |= item->mats[m];

        if (!any)
            return 0;                           /* a reward, not craftable */
    }

    if (progress.dungeonsCleared < item->minCleared)
        return 0;
    if (item->reqFlag >= 0 && !adv.flags[item->reqFlag])
        return 0;
    if (item->reqBoss >= 0 && !adv.optBoss[item->reqBoss])
        return 0;

    return 1;
}

void craftingMenu(void)
{
    while (1)
    {
        int kind[ITEM_COUNT + G_COUNT];     /* >=0 item id, <0 gadget -(g+1) */
        int count = 0;

        printf("\n=================================\n");
        printf("       THE FORGE: CRAFTING\n");
        printf("=================================\n");
        printf("Gold: %d   Materials: ", player.gold);
        for (int m = 0; m < MAT_COUNT; m++)
            printf("%s %d%s", materialName(m), adv.materials[m], m < MAT_COUNT - 1 ? ", " : "\n");
        printf("\n");

        for (int id = 1; id < ITEM_COUNT; id++)
        {
            const Item *item = getItem(id);

            if (!recipeVisible(item))
                continue;

            kind[count++] = id;
            printf("%2d. ", count);
            describe(item);
            printf(adv.owned[id] ? "   [owned]\n" : "\n");
            printf("      needs: ");
            printCost(item->mats, item->gold);
            printf(canAfford(item->mats, item->gold) ? "   (you can craft this)\n" : "\n");
        }

        for (int g = 0; g < G_COUNT; g++)
        {
            if (progress.dungeonsCleared < gadgets[g].minCleared)
                continue;

            kind[count++] = -(g + 1);
            printf("%2d. %s\n", count, gadgets[g].name);
            printf("      needs: ");
            printCost(gadgets[g].mats, gadgets[g].gold);
            printf(canAfford(gadgets[g].mats, gadgets[g].gold) ? "   (you can craft this)\n" : "\n");
        }

        printf("\nEnter a number to craft, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > count)
        {
            printf("Invalid choice.\n");
            continue;
        }

        int what = kind[choice - 1];

        if (what >= 0)
        {
            const Item *item = getItem(what);

            if (adv.owned[what])
                printf("You already own %s.\n", item->name);
            else if (!canAfford(item->mats, item->gold))
                printf("You don't have enough materials or gold.\n");
            else
            {
                pay(item->mats, item->gold);
                adv.stats[ST_CRAFTED]++;
                grantItem(what);
            }
        }
        else
        {
            int g = -what - 1;

            if (!canAfford(gadgets[g].mats, gadgets[g].gold))
                printf("You don't have enough materials or gold.\n");
            else
            {
                pay(gadgets[g].mats, gadgets[g].gold);
                adv.stats[ST_CRAFTED]++;

                if (g == G_POTION) player.potions++;
                if (g == G_MANA)   player.manaPotions++;
                if (g == G_BOMB)   player.fireBombs++;
                if (g == G_SMOKE)  player.smokeBombs++;
                if (g == G_ELIXIR) player.elixirs++;

                printf("You crafted a %s!\n", gadgets[g].name);
            }
        }
    }
}
