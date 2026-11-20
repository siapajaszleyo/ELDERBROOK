#ifndef FORGE_H
#define FORGE_H

/* =========================================================
   FORGE.H  —  Phase 4: the nested blacksmith
   ---------------------------------------------------------
   Two deeper screens reached from Brokk's forge:
     * ENCHANTING   — bind a permanent rune to your weapon or armour,
                      paid in materials + gold. One rune per slot;
                      re-enchanting replaces the old rune cleanly.
     * RELIC FORGE  — craft a relic from rare materials + Covenant
                      Creds. Gated behind the Covenant's Pactbearer
                      rank (see balance.h), per the Phase 3 design.
   Tuning knobs live in balance.h.
   ========================================================= */

/* Weapon enchants start at 1; armour enchants continue the same list so
   a single id space is saved per slot. 0 always means "no enchant". */
typedef enum
{
    ENC_NONE = 0,
    /* weapon runes */
    ENC_W_KEEN,        /* +8 ATK                        */
    ENC_W_FLAME,       /* +6 ATK, +6 CRIT               */
    ENC_W_VAMPIRE,     /* +5 ATK, +20 max HP            */
    /* armour runes */
    ENC_A_STURDY,      /* +6 DEF                        */
    ENC_A_WARD,        /* +4 DEF, +25 max HP            */
    ENC_A_ARCANE,      /* +3 DEF, +20 max MP            */
    ENC_COUNT
} EnchantKind;

const char *enchantName(int id);

/* The two nested forge screens (called from the blacksmith in hub.c). */
void forgeEnchantMenu(void);
void forgeRelicMenu(void);

#endif
