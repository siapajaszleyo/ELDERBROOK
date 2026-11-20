#ifndef CLASS_H
#define CLASS_H

#include "game.h"
#include "combat.h"

/* =========================================================
   CHARACTER 2.0 — Classes, Disciplines, Relics
   ========================================================= */

/* --- Classes --- */
typedef enum
{
    CLASS_NONE = 0,
    CLASS_KNIGHT,       /* Defense, counterattacks, dueling, heavy weapons */
    CLASS_ARCANIST,      /* Mana management and elemental magic */
    CLASS_ROGUE,         /* Speed, crits, evasion, poison, exploiting weakness */
    CLASS_RANGER,        /* Preparation, ranged attacks, traps */
    CLASS_WARLOCK,       /* Trading health and risk for supernatural power */
    CLASS_GAMBLER,       /* Risk/reward, probability, wagers, chance */
    CLASS_COUNT
} ClassKind;

/* --- Disciplines (3 per class) --- */
typedef enum
{
    DISC_NONE = 0,

    /* Knight */
    DISC_VANGUARD, DISC_DUELIST, DISC_WARLORD,

    /* Arcanist */
    DISC_PYROMANCER, DISC_CRYOMANCER, DISC_STORMCALLER,

    /* Rogue */
    DISC_ASSASSIN, DISC_TRICKSTER, DISC_POISONER,

    /* Ranger */
    DISC_ARCHER, DISC_HUNTER, DISC_TRAPPER,

    /* Warlock */
    DISC_BLOOD, DISC_SHADOW, DISC_PACT,

    /* Gambler */
    DISC_FORTUNE, DISC_RISK, DISC_DEALER,

    DISC_COUNT
} Discipline;

/* --- Class Abilities (3 per discipline, tier 1-3) --- */
/* Each ability is indexed: DISC_* * 3 + tier(0,1,2) */
enum
{
    /* Knight / Vanguard: Guard -> Shield Bash -> Iron Fortress */
    AB_GUARD, AB_SHIELD_BASH, AB_IRON_FORTRESS,

    /* Knight / Duelist: Riposte -> Precision -> Execution */
    AB_RIPOSTE, AB_PRECISION, AB_EXECUTION,

    /* Knight / Warlord: War Cry -> Command -> Warlord's Might */
    AB_WAR_CRY, AB_COMMAND, AB_WARLORDS_MIGHT,

    /* Arcanist / Pyromancer: Ignite -> Fireball -> Inferno */
    AB_IGNITE, AB_PYRO_FIREBALL, AB_INFERNO,

    /* Arcanist / Cryomancer: Frostbolt -> Freeze -> Absolute Zero */
    AB_FROSTBOLT, AB_FREEZE_SPELL, AB_ABSOLUTE_ZERO,

    /* Arcanist / Stormcaller: Spark -> Lightning Storm -> Tempest */
    AB_SPARK, AB_LIGHTNING_STORM, AB_TEMPEST,

    /* Rogue / Assassin: Bleed -> Backstab -> Execute */
    AB_BLEED, AB_BACKSTAB, AB_ASSASSIN_EXECUTE,

    /* Rogue / Trickster: Feint -> Disorient -> Shadow Step */
    AB_FEINT, AB_DISORIENT, AB_SHADOW_STEP,

    /* Rogue / Poisoner: Envenom -> Toxic Cloud -> Plague */
    AB_ENVENOM, AB_TOXIC_CLOUD, AB_PLAGUE,

    /* Ranger / Archer: Aim Shot -> Piercing Arrow -> Rain of Arrows */
    AB_AIM_SHOT, AB_PIERCING_ARROW, AB_RAIN_ARROWS,

    /* Ranger / Hunter: Mark Prey -> Hunter's Focus -> Devastating Shot */
    AB_MARK_PREY, AB_HUNTERS_FOCUS, AB_DEVASTATING_SHOT,

    /* Ranger / Trapper: Set Trap -> Snare -> Grand Trap */
    AB_SET_TRAP, AB_SNARE, AB_GRAND_TRAP,

    /* Warlock / Blood: Sacrifice -> Blood Magic -> Blood Pact */
    AB_SACRIFICE, AB_BLOOD_MAGIC, AB_BLOOD_PACT,

    /* Warlock / Shadow: Shadow Bolt -> Shadow Veil -> Soul Drain */
    AB_SHADOW_BOLT, AB_SHADOW_VEIL, AB_SOUL_DRAIN,

    /* Warlock / Pact: Dark Pact -> Forbidden Power -> Pact Summon */
    AB_DARK_PACT, AB_FORBIDDEN_POWER, AB_PACT_SUMMON,

    /* Gambler / Fortune: Lucky Strike -> Wild Card -> Jackpot */
    AB_LUCKY_STRIKE, AB_WILD_CARD, AB_JACKPOT,

    /* Gambler / Risk: All In -> Double Down -> High Roller */
    AB_ALL_IN, AB_DOUBLE_DOWN, AB_HIGH_ROLLER,

    /* Gambler / Dealer: Card Sharp -> Stacked Deck -> Dealer's Choice */
    AB_CARD_SHARP, AB_STACKED_DECK, AB_DEALERS_CHOICE,

    AB_COUNT
};

/* --- Relics --- */
typedef enum
{
    RELIC_NONE = 0,
    RELIC_EYE_OF_ORACLE,     /* reveals/improves enemy intent info */
    RELIC_BLOODSTONE,        /* selected abilities can consume HP instead of mana */
    RELIC_KINGS_COIN,        /* modifies gambling outcomes */
    RELIC_FROZEN_HEART,      /* strengthens freeze, fire tradeoff */
    RELIC_BLACK_COMPASS,     /* exploration and secret-room discovery */
    RELIC_DRAGON_TOOTH,      /* adds physical component to fire/dragon abilities */
    RELIC_COUNT
} RelicKind;

/* --- Combat State Flags (per-fight, stored in combat.c) --- */
#define CSTATE_GUARDING       0x01   /* Knight/Vanguard: guarding this turn */
#define CSTATE_COUNTER_READY  0x02   /* Knight/Duelist: riposte active */
#define CSTATE_WARCRY         0x04   /* Knight/Warlord: war cry active */
#define CSTATE_MARKED         0x08   /* Ranger/Hunter: enemy is marked */
#define CSTATE_TRAP          0x10   /* Ranger/Trapper: trap is set */
#define CSTATE_FEINTING      0x20   /* Rogue/Trickster: feint active */
#define CSTATE_SHADOW_VEIL   0x40   /* Warlock/Shadow: shadow veil active */

/* --- Enemy Telegraph State --- */
typedef enum
{
    TELE_NONE = 0,
    TELE_HEAVY,        /* next attack will be strong */
    TELE_SPECIAL,      /* next attack uses a special ability */
    TELE_DEFENSIVE,    /* enemy is defending/charging */
    TELE_ANGRY         /* enemy is enraged about something */
} Telegraph;

/* --- Class Data Access --- */
const char *className(ClassKind c);
const char *classDescription(ClassKind c);
const char *disciplineName(Discipline d);
const char *disciplineDescription(Discipline d);
const char *abilityName(int ab);
const char *abilityDescription(int ab);
int abilityManaCost(int ab);
Discipline classDiscipline(ClassKind c, int index);  /* index 0-2 */
ClassKind disciplineClass(Discipline d);

/* Which discipline does this ability belong to? */
Discipline abilityDiscipline(int ab);

/* What tier (1-3) is this ability? */
int abilityTier(int ab);

/* Class starting stat modifiers */
int classBaseHealth(ClassKind c);
int classBaseMana(ClassKind c);
int classBaseAttack(ClassKind c);
int classBaseDefense(ClassKind c);

/* Starting equipment id for this class (-1 = none) */
int classStartingWeapon(ClassKind c);
int classStartingArmor(ClassKind c);

/* Starting consumables */
int classStartingPotions(ClassKind c);
int classStartingManaPotions(ClassKind c);
int classStartingGold(ClassKind c);

/* Class-specific combat behaviour hooks */
int classEvasionBonus(ClassKind c);     /* percent bonus to dodge */
int classInitiativeBonus(ClassKind c);  /* percent chance to act first */

/* Does this class have a passive that modifies the gambler's games? */
int classGamblingBonus(ClassKind c);

/* Relic data */
const char *relicName(RelicKind r);
const char *relicDescription(RelicKind r);

/* Grant a relic to the player (equips it, replacing any current relic). */
void grantRelic(RelicKind r);

/* --- Class Selection (interactive) --- */
/* Shows the class picker, returns chosen class. */
ClassKind chooseClassScreen(void);

/* --- Discipline Selection (interactive) --- */
Discipline chooseDisciplineScreen(ClassKind c);

/* --- Apply starting loadout for a class+discipline --- */
void applyClassLoadout(ClassKind c, Discipline d);

/* --- Ability Menu (interactive, in combat) --- */
/* Shows class abilities, returns 1 if an ability was used, 0 if not. */
int classAbilityMenu(int *combatState, Telegraph telegraph,
                      int playerActingFirst,
                      int enemyIsFrozen, int enemyIsGuarded,
                      int enemyHPPercent);

/* --- Check if player has a specific ability --- */
int playerHasAbility(int ab);

/* --- Apply relic effects --- */
/* Returns modified damage after relic effects */
int relicOnDealDamage(int damage, int isFireOrDragon);
/* Returns modified damage taken after relic effects */
int relicOnTakeDamage(int damage);
/* Returns 1 if Bloodstone allows HP-cost instead of mana */
int relicBloodstoneActive(void);
/* Returns gambling modifier (add to win% chance) */
int relicGamblingBonus(void);
/* Returns secret-find bonus (add to discovery chance%) */
int relicSecretBonus(void);
/* Returns freeze bonus (add to freeze chance%) */
int relicFreezeBonus(void);
/* Returns 1 if Frozen Heart fire-tradeoff is active */
int relicFrozenHeartFirePenalty(void);

/* --- Enemy Telegraph System --- */
/* Roll a telegraph hint for the enemy's next move */
Telegraph rollTelegraph(const Enemy *enemy);
const char *telegraphText(Telegraph t);

#endif
