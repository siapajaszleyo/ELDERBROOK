#ifndef WEAPONS_H
#define WEAPONS_H

/* =========================================================
   WEAPONS.H  —  Weapon types & movesets (Expanded Edition)
   ---------------------------------------------------------
   Every weapon the hero can wield belongs to a WeaponType. The type
   decides the MOVESET shown on the "Attack" line in combat, so a staff
   fights nothing like an axe. This module is pure data/labels; the
   mechanics live in combat.c next to the damage helpers.
   ========================================================= */

typedef enum
{
    WT_SWORD = 0,   /* balanced: reliable slash + armour-breaking blow   */
    WT_DAGGER,      /* fast: quick low-commit stab + twin-hit flurry     */
    WT_STAFF,       /* arcane: strikes scale with mana, can channel      */
    WT_AXE,         /* heavy: hard chop + reckless cleave                */
    WT_BOW,         /* ranged: safe shot (uncounterable) + aimed shot    */
    WT_BLADE,       /* elemental: flowing cut + guaranteed elemental burst*/
    WT_COUNT
} WeaponType;

/* Each weapon type offers exactly this many moves on the Attack line. */
#define WEAPON_MOVES 2

/* Classify an equipment id into a weapon type. Non-weapons / unknown
   ids fall back to WT_SWORD so combat always has a valid moveset. */
WeaponType weaponTypeOf(int itemId);

const char *weaponTypeName(WeaponType t);
const char *weaponTypeTagline(WeaponType t);

/* move index is 0 or 1 (see WEAPON_MOVES). */
const char *weaponMoveName(WeaponType t, int move);
const char *weaponMoveDesc(WeaponType t, int move);

#endif
