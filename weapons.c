#include "weapons.h"
#include "items.h"

/* ---------------------------------------------------------
   Which weapon type is each equipment id?
   The index matches the equipment table in items.c. Only the
   SLOT_WEAPON ids matter; everything else returns WT_SWORD and is
   never actually used as a weapon.
   --------------------------------------------------------- */
WeaponType weaponTypeOf(int itemId)
{
    switch (itemId)
    {
    case 1:  return WT_SWORD;   /* Rusty Sword            */
    case 2:  return WT_SWORD;   /* Iron Sword             */
    case 3:  return WT_SWORD;   /* Knight Sword           */
    case 4:  return WT_BLADE;   /* Flame Blade            */
    case 5:  return WT_BLADE;   /* Frost Blade            */
    case 6:  return WT_BLADE;   /* Dragon Blade           */
    case 7:  return WT_SWORD;   /* Legendary Sword        */
    case 14: return WT_BLADE;   /* Forgotten Blade        */
    case 15: return WT_SWORD;   /* Pirate King's Cutlass  */
    case 16: return WT_DAGGER;  /* Duelist Rapier         */
    case 17: return WT_AXE;     /* Executioner's Axe      */
    case 18: return WT_STAFF;   /* Spellblade             */
    case 19: return WT_DAGGER;  /* Bloodfang              */
    case 20: return WT_SWORD;   /* Gambler's Blade        */
    case 26: return WT_BOW;     /* Hunter's Longbow       */
    case 27: return WT_STAFF;   /* Runed Staff            */
    default: return WT_SWORD;
    }
}

const char *weaponTypeName(WeaponType t)
{
    switch (t)
    {
    case WT_SWORD:  return "Sword";
    case WT_DAGGER: return "Dagger";
    case WT_STAFF:  return "Staff";
    case WT_AXE:    return "Axe";
    case WT_BOW:    return "Bow";
    case WT_BLADE:  return "Elemental Blade";
    default:        return "Weapon";
    }
}

const char *weaponTypeTagline(WeaponType t)
{
    switch (t)
    {
    case WT_SWORD:  return "Balanced steel — dependable in any hand.";
    case WT_DAGGER: return "Fast and vicious — strike before they react.";
    case WT_STAFF:  return "Arcane focus — your blows feed on mana.";
    case WT_AXE:    return "Brutal and heavy — trade safety for raw force.";
    case WT_BOW:    return "Ranged — hit from a distance, hard to counter.";
    case WT_BLADE:  return "Enchanted edge — unleash its element at will.";
    default:        return "";
    }
}

static const char *moveNames[WT_COUNT][WEAPON_MOVES] =
{
    /* WT_SWORD  */ { "Slash",        "Guard Break"      },
    /* WT_DAGGER */ { "Quick Stab",   "Flurry"           },
    /* WT_STAFF  */ { "Arcane Strike","Channel Power"    },
    /* WT_AXE    */ { "Chop",         "Reckless Cleave"  },
    /* WT_BOW    */ { "Shoot",        "Aimed Shot"       },
    /* WT_BLADE  */ { "Flowing Cut",  "Elemental Burst"  }
};

static const char *moveDescs[WT_COUNT][WEAPON_MOVES] =
{
    /* WT_SWORD  */ { "A clean, reliable blow.",
                      "Weaker hit, but cracks the enemy's armour for the fight." },
    /* WT_DAGGER */ { "A fast jab with a high crit chance.",
                      "Two rapid strikes in one turn." },
    /* WT_STAFF  */ { "A focused blow boosted by your current mana.",
                      "Spend 10 mana for a heavy strike that pierces defence." },
    /* WT_AXE    */ { "A heavy chop — more damage, wild swings.",
                      "A huge blow (can miss) that ignores much of their armour." },
    /* WT_BOW    */ { "A safe shot from range — cannot be countered.",
                      "Take aim for extra damage and crit (small miss chance)." },
    /* WT_BLADE  */ { "A smooth cutting stroke.",
                      "Release the blade's element for bonus damage + its effect." }
};

const char *weaponMoveName(WeaponType t, int move)
{
    if (t < 0 || t >= WT_COUNT || move < 0 || move >= WEAPON_MOVES)
        return "Attack";
    return moveNames[t][move];
}

const char *weaponMoveDesc(WeaponType t, int move)
{
    if (t < 0 || t >= WT_COUNT || move < 0 || move >= WEAPON_MOVES)
        return "";
    return moveDescs[t][move];
}
