#include <stdio.h>

#include "forge.h"
#include "adventure.h"
#include "items.h"
#include "class.h"
#include "guild.h"
#include "balance.h"
#include "input.h"

/* =========================================================
   ENCHANTS
   Each rune gives a flat, permanent bonus to the hero's stats
   (just like Brokk's sharpening). The bonus is applied to the
   live stats the moment you forge it and is already baked into
   the numbers that get saved, so nothing is re-applied on load.
   The slot's enchant id is remembered so we can show it and so a
   replacement rune can cleanly undo the previous one first.
   ========================================================= */

typedef struct
{
    const char *name;
    int slot;                 /* SLOT_WEAPON or SLOT_ARMOR */
    int attack, defense, health, mana, crit;
    int mats[MAT_COUNT];
    int gold;
    const char *blurb;
} Enchant;

/* Indexed by EnchantKind. ENC_NONE is a placeholder. */
static const Enchant enchants[ENC_COUNT] =
{
    { "(none)", SLOT_NONE, 0,0,0,0,0, {0}, 0, "" },

    /* --- weapon runes --- */
    { "Keen Edge",      SLOT_WEAPON, 8,0, 0, 0,0, {6,0,0,2,0,0,0},  80,
      "A honed rune: +8 attack." },
    { "Flaming Rune",   SLOT_WEAPON, 6,0, 0, 0,6, {0,0,2,4,0,0,0}, 120,
      "Searing script: +6 attack, +6% crit." },
    { "Vampiric Sigil", SLOT_WEAPON, 5,0,20, 0,0, {0,0,0,2,0,3,0}, 150,
      "A hungry rune: +5 attack, +20 max HP." },

    /* --- armour runes --- */
    { "Sturdy Plating", SLOT_ARMOR,  0,6, 0, 0,0, {8,0,0,0,0,0,0},  90,
      "Layered steel: +6 defense." },
    { "Warding Glyph",  SLOT_ARMOR,  0,4,25, 0,0, {0,0,2,4,0,0,0}, 120,
      "A protective ward: +4 defense, +25 max HP." },
    { "Arcane Weave",   SLOT_ARMOR,  0,3, 0,20,0, {0,0,0,3,0,0,1}, 150,
      "Woven mana-threads: +3 defense, +20 max MP." }
};

const char *enchantName(int id)
{
    if (id <= ENC_NONE || id >= ENC_COUNT)
        return "(none)";
    return enchants[id].name;
}

/* Add (sign = +1) or remove (sign = -1) a rune's bonuses. */
static void applyEnchant(int id, int sign)
{
    if (id <= ENC_NONE || id >= ENC_COUNT)
        return;

    const Enchant *e = &enchants[id];

    if (adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK)
        player.maxMana += sign * e->attack;
    else
        player.attack += sign * e->attack;
    player.defense   += sign * e->defense;
    player.maxHealth += sign * e->health;
    player.maxMana   += sign * e->mana;
    player.critChance+= sign * e->crit;
    if (player.critChance > 100) player.critChance = 100;

    if (player.health > player.maxHealth) player.health = player.maxHealth;
    if (player.mana   > player.maxMana)   player.mana   = player.maxMana;
    if (player.critChance < 0)            player.critChance = 0;
}

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

    if (first && gold == 0)
        printf("nothing");
}

void forgeEnchantMenu(void)
{
    while (1)
    {
        int list[ENC_COUNT];
        int count = 0;

        printf("\n=================================\n");
        printf("      THE FORGE: ENCHANTING\n");
        printf("=================================\n");
        printf("Gold: %d   Materials: ", player.gold);
        for (int m = 0; m < MAT_COUNT; m++)
            printf("%s %d%s", materialName(m), adv.materials[m],
                   m < MAT_COUNT - 1 ? ", " : "\n");

        printf("\nWeapon: %s", getItem(adv.weapon)->name);
        if (adv.weaponEnchant)
            printf("  [%s]", enchantName(adv.weaponEnchant));
        printf("\nArmor:  %s", adv.armor ? getItem(adv.armor)->name : "(nothing)");
        if (adv.armorEnchant)
            printf("  [%s]", enchantName(adv.armorEnchant));
        printf("\n\nOne rune per slot. Forging a new rune replaces the old one.\n");
        printf("A rune stays with the slot even if you change gear.\n\n");

        for (int id = 1; id < ENC_COUNT; id++)
        {
            const Enchant *e = &enchants[id];

            /* can only enchant a slot you actually have gear in */
            if (e->slot == SLOT_ARMOR && adv.armor == 0)
                continue;

            list[count++] = id;
            printf("%d. %s  (%s)\n", count,
                   e->name, e->slot == SLOT_WEAPON ? "weapon" : "armor");
            printf("     %s\n", e->blurb);
            printf("     needs: ");
            printCost(e->mats, e->gold);
            printf(canAfford(e->mats, e->gold) ? "   (affordable)\n" : "\n");
        }

        if (adv.armor == 0)
            printf("(Equip some armour to unlock armour runes.)\n");

        printf("\nEnter a number to forge a rune, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > count)
        {
            printf("Invalid choice.\n");
            continue;
        }

        int id = list[choice - 1];
        const Enchant *e = &enchants[id];
        int *slotEnc = (e->slot == SLOT_WEAPON) ? &adv.weaponEnchant
                                                : &adv.armorEnchant;

        if (*slotEnc == id)
        {
            printf("That rune is already bound to this slot.\n");
            continue;
        }

        if (!canAfford(e->mats, e->gold))
        {
            printf("You don't have enough materials or gold.\n");
            continue;
        }

        pay(e->mats, e->gold);

        if (*slotEnc != ENC_NONE)
        {
            printf("Brokk grinds off the old %s rune.\n", enchantName(*slotEnc));
            applyEnchant(*slotEnc, -1);
        }

        *slotEnc = id;
        applyEnchant(id, +1);
        adv.stats[ST_CRAFTED]++;

        printf("\n*** %s bound to your %s! ***\n", e->name,
               e->slot == SLOT_WEAPON ? "weapon" : "armor");
        printf("%s\n", e->blurb);
    }
}


/* =========================================================
   RELIC FORGE
   Gated behind the Covenant's Pactbearer rank. Each relic costs
   rare materials plus Covenant Creds. Forging grants the relic
   (replacing whatever relic you currently carry).
   ========================================================= */

typedef struct
{
    RelicKind relic;
    int mats[MAT_COUNT];
    int creds;                /* Covenant Creds */
} RelicRecipe;

static const RelicRecipe relicRecipes[] =
{
    { RELIC_EYE_OF_ORACLE, {0,0,0,4,0,0,2}, RELIC_CREDS_EYE },
    { RELIC_BLOODSTONE,    {0,0,3,0,0,4,0}, RELIC_CREDS_BLOODSTONE },
    { RELIC_KINGS_COIN,    {0,0,0,3,0,0,2}, RELIC_CREDS_COIN },
    { RELIC_FROZEN_HEART,  {0,0,0,6,0,0,0}, RELIC_CREDS_FROZEN },
    { RELIC_BLACK_COMPASS, {0,0,2,0,0,0,3}, RELIC_CREDS_COMPASS },
    { RELIC_DRAGON_TOOTH,  {0,0,0,0,4,0,2}, RELIC_CREDS_DRAGON }
};

#define RELIC_RECIPE_COUNT ((int)(sizeof relicRecipes / sizeof relicRecipes[0]))

void forgeRelicMenu(void)
{
    int rank = adv.guildRank[GUILD_COVENANT];

    if (rank < COVENANT_RELIC_RANK)
    {
        printf("\nBrokk lowers his voice. \"Relic-work? That's Covenant craft.\n");
        printf("Bring me their blessing \u2014 the rank of %s \u2014 and the rare\n",
               guildRankName(GUILD_COVENANT, COVENANT_RELIC_RANK));
        printf("stock to match, and I'll bind a relic for you.\"\n");
        printf("(You are currently %s with the Covenant.)\n",
               rank ? guildRankName(GUILD_COVENANT, rank) : "not a member");
        return;
    }

    while (1)
    {
        int creds = adv.guildCreds[GUILD_COVENANT];

        printf("\n=================================\n");
        printf("      THE FORGE: RELIC WORK\n");
        printf("=================================\n");
        printf("Covenant Creds: %d\n", creds);
        printf("Materials: ");
        for (int m = 0; m < MAT_COUNT; m++)
            printf("%s %d%s", materialName(m), adv.materials[m],
                   m < MAT_COUNT - 1 ? ", " : "\n");
        printf("Relic carried: %s\n\n", relicName((RelicKind)adv.relic));
        printf("You carry one relic at a time; forging a new one replaces it.\n\n");

        for (int i = 0; i < RELIC_RECIPE_COUNT; i++)
        {
            const RelicRecipe *r = &relicRecipes[i];
            int affordMats = canAfford(r->mats, 0);
            int affordCreds = creds >= r->creds;

            printf("%d. %s%s\n", i + 1, relicName(r->relic),
                   adv.relic == (int)r->relic ? "   [carried]" : "");
            printf("     %s\n", relicDescription(r->relic));
            printf("     needs: ");
            printCost(r->mats, 0);
            printf(", %d Creds", r->creds);
            printf((affordMats && affordCreds) ? "   (affordable)\n" : "\n");
        }

        printf("\nEnter a number to forge a relic, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > RELIC_RECIPE_COUNT)
        {
            printf("Invalid choice.\n");
            continue;
        }

        const RelicRecipe *r = &relicRecipes[choice - 1];

        if (adv.relic == (int)r->relic)
        {
            printf("You already carry the %s.\n", relicName(r->relic));
            continue;
        }

        if (!canAfford(r->mats, 0))
        {
            printf("You don't have the rare materials for that.\n");
            continue;
        }

        if (adv.guildCreds[GUILD_COVENANT] < r->creds)
        {
            printf("You need %d Covenant Creds for that.\n", r->creds);
            continue;
        }

        pay(r->mats, 0);
        adv.guildCreds[GUILD_COVENANT] -= r->creds;
        adv.stats[ST_CRAFTED]++;

        grantRelic(r->relic);
    }
}
