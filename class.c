#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "class.h"
#include "adventure.h"
#include "items.h"
#include "skills.h"
#include "input.h"

/* =========================================================
   CLASS DATA TABLES
   ========================================================= */

typedef struct
{
    const char *name;
    const char *description;
    int baseHP, baseMP, baseAtk, baseDef;
    int startWeapon, startArmor;
    int startPotions, startManaPots, startGold;
    int evasionBonus, initBonus, gambleBonus;
    Discipline discs[3];
} ClassData;

static const ClassData classData[CLASS_COUNT] =
{
    /* CLASS_NONE */
    { "None", "No class.", 100, 50, 20, 10, 1, 0, 3, 2, 100, 0, 0, 0,
      { DISC_NONE, DISC_NONE, DISC_NONE } },

    /* CLASS_KNIGHT */
    { "Knight",
      "Masters of defense and counterattacks. Knights endure what would break others,"
      " and punish those who overcommit.",
      120, 40, 18, 18, 2, 9, 2, 1, 80, 0, 5, 0,
      { DISC_VANGUARD, DISC_DUELIST, DISC_WARLORD } },

    /* CLASS_ARCANIST */
    { "Arcanist",
      "Wielders of elemental magic. Arcanists manage mana carefully and unleash"
      " devastating spells when the moment is right.",
      80, 70, 14, 8, 27, 10, 1, 3, 120, 0, 0, 0,
      { DISC_PYROMANCER, DISC_CRYOMANCER, DISC_STORMCALLER } },

    /* CLASS_ROGUE */
    { "Rogue",
      "Fast, deadly, and never where the enemy expects. Rogues build advantage,"
      " then strike with lethal precision.",
      85, 45, 22, 8, 1, 11, 2, 2, 90, 15, 5, 0,
      { DISC_ASSASSIN, DISC_TRICKSTER, DISC_POISONER } },

    /* CLASS_RANGER */
    { "Ranger",
      "Masters of preparation and ranged combat. Rangers set the terms of"
      " engagement before the enemy can react.",
      90, 50, 20, 10, 26, 8, 2, 2, 100, 5, 5, 0,
      { DISC_ARCHER, DISC_HUNTER, DISC_TRAPPER } },

    /* CLASS_WARLOCK */
    { "Warlock",
      "Those who trade health and risk for supernatural power. Warlocks walk a"
      " razor edge between devastation and self-destruction.",
      85, 55, 16, 10, 1, 11, 1, 2, 80, 0, 0, 0,
      { DISC_BLOOD, DISC_SHADOW, DISC_PACT } },

    /* CLASS_GAMBLER */
    { "Gambler",
      "Fortune's favourite and its plaything. Gamblers wager everything on chance,"
      " and sometimes the odds bend in their favour.",
      90, 45, 18, 10, 1, 12, 2, 2, 100, 0, 0, 15,
      { DISC_FORTUNE, DISC_RISK, DISC_DEALER } }
};

typedef struct
{
    const char *name;
    const char *description;
    ClassKind   parentClass;
} DiscData;

static const DiscData discData[DISC_COUNT] =
{
    /* DISC_NONE */
    { "None", "No discipline.", CLASS_NONE },

    /* Knight */
    { "Vanguard",   "Hold the line. Guard allies, shield bash, become an iron fortress.",    CLASS_KNIGHT },
    { "Duelist",    "One-on-one mastery. Riposte, precision strikes, and lethal executions.", CLASS_KNIGHT },
    { "Warlord",    "Command the battlefield. War cries, commands, and overwhelming might.",  CLASS_KNIGHT },

    /* Arcanist */
    { "Pyromancer",  "Burn it all. Ignite, fireball, unleash inferno.",                        CLASS_ARCANIST },
    { "Cryomancer",  "Freeze and shatter. Frostbolt, freeze, absolute zero.",                  CLASS_ARCANIST },
    { "Stormcaller", "Command the storm. Spark, lightning storm, tempest.",                     CLASS_ARCANIST },

    /* Rogue */
    { "Assassin",   "Strike from shadow. Bleed, backstab, execute.",                           CLASS_ROGUE },
    { "Trickster",  "Confuse and misdirect. Feint, disorient, shadow step.",                   CLASS_ROGUE },
    { "Poisoner",   "Death by a thousand drops. Envenom, toxic cloud, plague.",               CLASS_ROGUE },

    /* Ranger */
    { "Archer",     "Deadly at range. Aim shot, piercing arrow, rain of arrows.",              CLASS_RANGER },
    { "Hunter",     "Track and destroy. Mark prey, focus, devastating shot.",                  CLASS_RANGER },
    { "Trapper",     "Control the field. Set trap, snare, grand trap.",                        CLASS_RANGER },

    /* Warlock */
    { "Blood",      "Power through pain. Sacrifice, blood magic, blood pact.",                 CLASS_WARLOCK },
    { "Shadow",     "Darkness is your ally. Shadow bolt, shadow veil, soul drain.",            CLASS_WARLOCK },
    { "Pact",       "Bargain with the beyond. Dark pact, forbidden power, pact summon.",      CLASS_WARLOCK },

    /* Gambler */
    { "Fortune",    "Luck favours the bold. Lucky strike, wild card, jackpot.",                CLASS_GAMBLER },
    { "Risk",       "Double or nothing. All in, double down, high roller.",                    CLASS_GAMBLER },
    { "Dealer",     "The house always wins. Card sharp, stacked deck, dealer's choice.",       CLASS_GAMBLER }
};

typedef struct
{
    const char *name;
    const char *description;
    int         manaCost;
} AbilityData;

static const AbilityData abilityData[AB_COUNT] =
{
    /* Knight / Vanguard */
    { "Guard",          "Halve the next hit you take and gain +5 defense for 1 turn. (6 mana)",    6 },
    { "Shield Bash",    "Strike with your shield: deals damage based on your defense and stuns the enemy 1 turn. (10 mana)", 10 },
    { "Iron Fortress",  "For 3 turns, all damage you take is halved. (20 mana)",                    20 },

    /* Knight / Duelist */
    { "Riposte",        "Enter a defensive stance: the next hit you take triggers a powerful counter. (8 mana)", 8 },
    { "Precision",      "Your next attack cannot miss and has +25% critical chance. (10 mana)",     10 },
    { "Execution",      "A lethal strike that deals massive bonus damage to enemies below 30% HP. (22 mana)", 22 },

    /* Knight / Warlord */
    { "War Cry",        "Boost your attack by 30% for 2 turns. (8 mana)",                          8 },
    { "Command",        "Force the enemy to waste its next turn. (14 mana)",                        14 },
    { "Warlord's Might","Your next attack deals double damage and cannot be blocked. (25 mana)",   25 },

    /* Arcanist / Pyromancer */
    { "Ignite",         "Set the enemy ablaze: deals fire damage and burns for 3 turns. (8 mana)",  8 },
    { "Pyro Fireball",  "A massive fireball: heavy fire damage, stronger than regular fireball. (14 mana)", 14 },
    { "Inferno",        "Engulf the battlefield: devastating fire damage to the enemy. (30 mana)",   30 },

    /* Arcanist / Cryomancer */
    { "Frostbolt",      "A bolt of ice: damage plus 35% chance to freeze the enemy 1 turn. (8 mana)", 8 },
    { "Freeze Spell",   "Freeze the enemy solid for 2 turns. (16 mana)",                          16 },
    { "Absolute Zero",  "Devastating ice damage. If the enemy is frozen, deals triple. (28 mana)",  28 },

    /* Arcanist / Stormcaller */
    { "Spark",          "A quick bolt of lightning: moderate damage, ignores defense. (6 mana)",     6 },
    { "Lightning Storm","Call down a storm: 3 bolts of lightning over 3 turns. (18 mana)",        18 },
    { "Tempest",        "Unleash the full tempest: massive damage, ignores defense and armor. (32 mana)", 32 },

    /* Rogue / Assassin */
    { "Bleed",          "Slash to inflict bleeding: enemy takes damage each turn for 3 turns. (6 mana)", 6 },
    { "Backstab",       "If you attack before the enemy acts, deal double damage. (10 mana)",       10 },
    { "Assassin Execute","A lethal blow: if enemy is below 25% HP, instantly defeat it. (20 mana)", 20 },

    /* Rogue / Trickster */
    { "Feint",          "Fake an attack: the enemy's next strike misses, and you gain advantage. (6 mana)", 6 },
    { "Disorient",      "Confuse the enemy: 50% chance it hurts itself instead of attacking. (12 mana)", 12 },
    { "Shadow Step",    "Vanish and reappear: you dodge all attacks for 2 turns. (18 mana)",        18 },

    /* Rogue / Poisoner */
    { "Envenom",        "Coat your blade in poison: attacks deal extra poison damage for 3 turns. (6 mana)", 6 },
    { "Toxic Cloud",    "Release a cloud of poison: damage now and over 3 turns. (14 mana)",        14 },
    { "Plague",         "A virulent plague: heavy damage now, massive damage over 3 turns. (24 mana)", 24 },

    /* Ranger / Archer */
    { "Aim Shot",       "Take careful aim: guaranteed hit with +50% critical chance. (8 mana)",     8 },
    { "Piercing Arrow", "An arrow that pierces armor: ignores defense. (12 mana)",                  12 },
    { "Rain of Arrows", "A volley of arrows: 3 hits of moderate damage. (20 mana)",                20 },

    /* Ranger / Hunter */
    { "Mark Prey",      "Mark the enemy: all attacks against it deal +30% damage for 3 turns. (8 mana)", 8 },
    { "Hunter's Focus", "Focus your aim: +50% critical chance and +30% damage for 2 turns. (12 mana)", 12 },
    { "Devastating Shot","A single devastating shot: massive damage, guaranteed crit if enemy is marked. (22 mana)", 22 },

    /* Ranger / Trapper */
    { "Set Trap",       "Set a trap: the enemy's next attack triggers a counter-hit. (8 mana)",     8 },
    { "Snare",          "Snare the enemy: it loses its next 2 turns. (16 mana)",                  16 },
    { "Grand Trap",     "An elaborate trap: when triggered, deals heavy damage and stuns 2 turns. (24 mana)", 24 },

    /* Warlock / Blood */
    { "Sacrifice",      "Pay 15% of your HP to gain a surge of power: +50% damage next attack. (0 mana, costs HP)", 0 },
    { "Blood Magic",    "Pay 20% HP to deal heavy damage and heal for half the damage dealt. (0 mana, costs HP)", 0 },
    { "Blood Pact",     "Pay 30% HP: your next 3 attacks deal double damage and heal you for the damage dealt. (0 mana, costs HP)", 0 },

    /* Warlock / Shadow */
    { "Shadow Bolt",    "A bolt of dark energy: moderate damage, cannot be blocked. (8 mana)",      8 },
    { "Shadow Veil",    "Cloak yourself in shadow: 50% evasion for 2 turns. (12 mana)",            12 },
    { "Soul Drain",     "Drain the enemy's life: heavy damage and heal for the full amount. (22 mana)", 22 },

    /* Warlock / Pact */
    { "Dark Pact",      "Sacrifice 10% HP to gain 40% of max mana back. (0 mana, costs HP)",       0 },
    { "Forbidden Power", "Double your attack for 2 turns, but take 10% max HP damage per turn. (0 mana, costs HP)", 0 },
    { "Pact Summon",    "Summon a dark ally: deals damage equal to 3x your level each turn for 3 turns. (15 mana)", 15 },

    /* Gambler / Fortune */
    { "Lucky Strike",   "A strike with a 50% chance to deal double damage, otherwise normal. (6 mana)", 6 },
    { "Wild Card",      "Random effect: heal, damage, buff, or nothing. You never know! (8 mana)", 8 },
    { "Jackpot",        "3 independent 33% chances to deal big damage. All three hitting is devastating. (20 mana)", 20 },

    /* Gambler / Risk */
    { "All In",         "Wager 25% of your HP: if you win (55% chance), deal triple damage. Otherwise, lose the HP. (0 mana, costs HP)", 0 },
    { "Double Down",    "Double your current attack for 1 turn, but if you miss you take 20% max HP damage. (10 mana)", 10 },
    { "High Roller",    "For 3 turns, every attack has a 40% chance to deal double damage. (18 mana)", 18 },

    /* Gambler / Dealer */
    { "Card Sharp",     "Draw a card: Heart=heal, Diamond=gold, Club=+30% damage, Spade=enemy takes damage. (6 mana)", 6 },
    { "Stacked Deck",   "For 3 turns, all your gambling abilities have improved outcomes. (12 mana)", 12 },
    { "Dealer's Choice","The ultimate gamble: 5 random effects in one turn. Chaos guaranteed. (25 mana)", 25 }
};

/* --- Relic Data --- */
typedef struct
{
    const char *name;
    const char *description;
} RelicData;

static const RelicData relicData[RELIC_COUNT] =
{
    { "None",             "No relic." },
    { "Eye of the Oracle", "Reveals enemy telegraphs and improves enemy-intent information in combat." },
    { "Bloodstone",        "Selected abilities can consume HP instead of mana, at 1.5x the HP cost." },
    { "King's Coin",       "Improves all gambling outcomes by +15% win chance." },
    { "Frozen Heart",      "Strengthens freeze effects (+20% freeze chance), but fire damage you deal is reduced by 20%." },
    { "Black Compass",     "Increases secret-room discovery chance by +25% and reveals extra exploration hints." },
    { "Dragon Tooth",      "Adds physical bonus damage equal to your level to all fire and dragon abilities." }
};


/* =========================================================
   ACCESSOR FUNCTIONS
   ========================================================= */

const char *className(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return "None";
    return classData[c].name;
}

const char *classDescription(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return "";
    return classData[c].description;
}

const char *disciplineName(Discipline d)
{
    if (d <= DISC_NONE || d >= DISC_COUNT) return "None";
    return discData[d].name;
}

const char *disciplineDescription(Discipline d)
{
    if (d <= DISC_NONE || d >= DISC_COUNT) return "";
    return discData[d].description;
}

const char *abilityName(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return "?";
    return abilityData[ab].name;
}

const char *abilityDescription(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return "";
    return abilityData[ab].description;
}

int abilityManaCost(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return 0;
    return abilityData[ab].manaCost;
}

Discipline classDiscipline(ClassKind c, int index)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT || index < 0 || index >= 3)
        return DISC_NONE;
    return classData[c].discs[index];
}

ClassKind disciplineClass(Discipline d)
{
    if (d <= DISC_NONE || d >= DISC_COUNT) return CLASS_NONE;
    return discData[d].parentClass;
}

Discipline abilityDiscipline(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return DISC_NONE;
    /* Each discipline has 3 abilities starting at DISC_NONE+1 */
    int discIndex = ab / 3 + 1;   /* +1 because DISC_NONE=0 is skipped */
    if (discIndex >= DISC_COUNT) return DISC_NONE;
    return (Discipline)discIndex;
}

int abilityTier(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return 0;
    return (ab % 3) + 1;
}

int classBaseHealth(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 100;
    return classData[c].baseHP;
}

int classBaseMana(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 50;
    return classData[c].baseMP;
}

int classBaseAttack(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 20;
    return classData[c].baseAtk;
}

int classBaseDefense(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 10;
    return classData[c].baseDef;
}

int classStartingWeapon(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 1;
    return classData[c].startWeapon;
}

int classStartingArmor(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 0;
    return classData[c].startArmor;
}

int classStartingPotions(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 3;
    return classData[c].startPotions;
}

int classStartingManaPotions(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 2;
    return classData[c].startManaPots;
}

int classStartingGold(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 100;
    return classData[c].startGold;
}

int classEvasionBonus(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 0;
    return classData[c].evasionBonus;
}

int classInitiativeBonus(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 0;
    return classData[c].initBonus;
}

int classGamblingBonus(ClassKind c)
{
    if (c <= CLASS_NONE || c >= CLASS_COUNT) return 0;
    return classData[c].gambleBonus;
}

const char *relicName(RelicKind r)
{
    if (r <= RELIC_NONE || r >= RELIC_COUNT) return "None";
    return relicData[r].name;
}

const char *relicDescription(RelicKind r)
{
    if (r <= RELIC_NONE || r >= RELIC_COUNT) return "";
    return relicData[r].description;
}

void grantRelic(RelicKind r)
{
    if (r <= RELIC_NONE || r >= RELIC_COUNT)
        return;

    if (adv.relic == (int)r)
    {
        printf("\nYou already carry the %s.\n", relicName(r));
        return;
    }

    if (adv.relic != RELIC_NONE)
    {
        printf("\nYou set aside the %s and take up the %s instead.\n",
               relicName((RelicKind)adv.relic), relicName(r));
    }
    else
    {
        printf("\n*** You obtained a relic: %s! ***\n", relicName(r));
    }
    printf("%s\n", relicDescription(r));

    adv.relic = r;
    player.relic = r;
}


/* =========================================================
   CLASS & DISCIPLINE SELECTION SCREENS
   ========================================================= */

ClassKind chooseClassScreen(void)
{
    printf("\n=========================================\n");
    printf("     CHOOSE YOUR CLASS\n");
    printf("=========================================\n");
    printf("Your class defines who you are and how you fight.\n");
    printf("It is not just starting stats - it shapes every decision you make in combat.\n\n");

    for (int c = CLASS_KNIGHT; c < CLASS_COUNT; c++)
    {
        printf("%d. %-12s  HP:%d MP:%d ATK:%d DEF:%d\n",
               c, className(c),
               classBaseHealth(c), classBaseMana(c),
               classBaseAttack(c), classBaseDefense(c));
        printf("   %s\n\n", classDescription(c));
    }

    while (1)
    {
        printf("Enter the number of your class: ");
        int choice = readInt();

        if (choice >= CLASS_KNIGHT && choice < CLASS_COUNT)
            return (ClassKind)choice;

        printf("Invalid choice. Pick a class from %d to %d.\n", CLASS_KNIGHT, CLASS_COUNT - 1);
    }
}

Discipline chooseDisciplineScreen(ClassKind c)
{
    printf("\n=========================================\n");
    printf("     CHOOSE YOUR DISCIPLINE\n");
    printf("=========================================\n");
    printf("As a %s, you may follow one of three paths.\n", className(c));
    printf("Your discipline determines which abilities you can learn.\n");
    printf("You cannot learn abilities from other disciplines.\n\n");

    for (int i = 0; i < 3; i++)
    {
        Discipline d = classDiscipline(c, i);
        printf("%d. %-14s %s\n", i + 1, disciplineName(d), disciplineDescription(d));

        /* Show the 3 abilities in this discipline */
        int baseAb = (d - 1) * 3;   /* -1 because DISC_NONE=0 */
        printf("   Abilities: %s -> %s -> %s\n\n",
               abilityName(baseAb), abilityName(baseAb + 1), abilityName(baseAb + 2));
    }

    while (1)
    {
        printf("Enter the number of your discipline (1-3): ");
        int choice = readInt();

        if (choice >= 1 && choice <= 3)
            return classDiscipline(c, choice - 1);

        printf("Invalid choice.\n");
    }
}


/* =========================================================
   APPLY STARTING LOADOUT
   ========================================================= */

void applyClassLoadout(ClassKind c, Discipline d)
{
    player.maxHealth = classBaseHealth(c);
    player.health    = player.maxHealth;
    player.maxMana   = classBaseMana(c);
    player.mana      = player.maxMana;
    player.attack    = classBaseAttack(c);
    player.defense   = classBaseDefense(c);
    player.potions     = classStartingPotions(c);
    player.manaPotions = classStartingManaPotions(c);
    player.gold        = classStartingGold(c);

    /* Grant starting equipment */
    int weapon = classStartingWeapon(c);
    int armor  = classStartingArmor(c);

    /* Start with the Rusty Sword removed if class gets something else */
    adv.owned[ITEM_RUSTY_SWORD] = 0;

    if (weapon > 0)
    {
        adv.owned[weapon] = 1;
        adv.weapon = weapon;
        const Item *wItem = getItem(weapon);
        if (c == CLASS_ARCANIST || c == CLASS_WARLOCK)
            player.maxMana += wItem->attack;
        else
            player.attack += wItem->attack;
        player.defense    += wItem->defense;
        player.maxHealth  += wItem->health;
        player.health     += wItem->health;
        player.maxMana    += wItem->mana;
        player.mana       += wItem->mana;
        player.critChance += wItem->crit;
        player.weaponFx    = wItem->fx;
    }
    else
    {
        adv.owned[ITEM_RUSTY_SWORD] = 1;
        adv.weapon = ITEM_RUSTY_SWORD;
    }

    if (armor > 0)
    {
        adv.owned[armor] = 1;
        adv.armor = armor;
        const Item *aItem = getItem(armor);
        player.defense    += aItem->defense;
        player.maxHealth  += aItem->health;
        player.health     += aItem->health;
        player.maxMana    += aItem->mana;
        player.mana       += aItem->mana;
        player.critChance += aItem->crit;
        player.armorFx     = aItem->fx;
    }

    /* Grant the first ability of the chosen discipline */
    int firstAbility = (d - 1) * 3;
    if (firstAbility >= 0 && firstAbility < AB_COUNT)
    {
        adv.classAbilities[firstAbility] = 1;
        printf("\nYou have learned: %s!\n", abilityName(firstAbility));
    }

    /* Class-specific starting bonuses */
    if (c == CLASS_ROGUE)
        player.critChance += 10;
    if (c == CLASS_KNIGHT)
        player.healPower += 5;

    /* Phase 3: born a Rank-1 member of your origin's guild. */
    guildSeedHome(c);
}


/* =========================================================
   ABILITY QUERY
   ========================================================= */

int playerHasAbility(int ab)
{
    if (ab < 0 || ab >= AB_COUNT) return 0;
    return adv.classAbilities[ab];
}


/* =========================================================
   RELIC EFFECTS
   ========================================================= */

int relicOnDealDamage(int damage, int isFireOrDragon)
{
    if (adv.relic == RELIC_DRAGON_TOOTH && isFireOrDragon)
        damage += player.level;

    if (adv.relic == RELIC_FROZEN_HEART && isFireOrDragon)
        damage = damage * 80 / 100;  /* 20% reduction on fire */

    return damage;
}

int relicOnTakeDamage(int damage)
{
    /* Currently no relic reduces incoming damage directly */
    return damage;
}

int relicBloodstoneActive(void)
{
    return adv.relic == RELIC_BLOODSTONE;
}

int relicGamblingBonus(void)
{
    if (adv.relic == RELIC_KINGS_COIN) return 15;
    return 0;
}

int relicSecretBonus(void)
{
    if (adv.relic == RELIC_BLACK_COMPASS) return 25;
    return 0;
}

int relicFreezeBonus(void)
{
    if (adv.relic == RELIC_FROZEN_HEART) return 20;
    return 0;
}

int relicFrozenHeartFirePenalty(void)
{
    return adv.relic == RELIC_FROZEN_HEART;
}


/* =========================================================
   ENEMY TELEGRAPH SYSTEM
   ========================================================= */

Telegraph rollTelegraph(const Enemy *enemy)
{
    int roll = rand() % 100;

    /* Heavier hitters telegraph more often */
    if (enemy->health * 2 < enemy->maxHealth)
    {
        /* Low health = more aggressive telegraphs */
        if (roll < 30) return TELE_HEAVY;
        if (roll < 45) return TELE_SPECIAL;
        if (roll < 55) return TELE_ANGRY;
        return TELE_NONE;
    }

    if (roll < 15) return TELE_HEAVY;
    if (roll < 25) return TELE_SPECIAL;
    if (roll < 35) return TELE_DEFENSIVE;
    return TELE_NONE;
}

const char *telegraphText(Telegraph t)
{
    switch (t)
    {
    case TELE_HEAVY:      return "The enemy is gathering its strength for a powerful strike!";
    case TELE_SPECIAL:    return "The enemy is channeling a special ability!";
    case TELE_DEFENSIVE:  return "The enemy is adopting a defensive stance.";
    case TELE_ANGRY:      return "The enemy is enraged and fighting desperately!";
    default:              return NULL;
    }
}


/* =========================================================
   CLASS ABILITY MENU (COMBAT)
   This shows available class abilities and lets the player use one.
   Returns 1 if an ability was used (end player turn), 0 if not.
   combatState is a bitmask (CSTATE_* flags) that both reads and writes.
   ========================================================= */

int classAbilityMenu(int *combatState, Telegraph telegraph,
                      int playerActingFirst,
                      int enemyIsFrozen, int enemyIsGuarded,
                      int enemyHPPercent)
{
    (void)telegraph;
    (void)enemyIsFrozen;
    (void)enemyIsGuarded;

    /* List available abilities */
    /* list[AB_COUNT] should be large enough, but AB_COUNT may vary */
    int list[60];
    int count = 0;

    printf("\n--- Class Abilities ---\n");

    for (int ab = 0; ab < AB_COUNT; ab++)
    {
        if (!adv.classAbilities[ab]) continue;

        /* Check prerequisite: must know the previous tier in this discipline */
        int tier = abilityTier(ab);
        if (tier > 1 && !adv.classAbilities[ab - 1])
            continue;  /* shouldn't happen, but safety */

        list[count] = ab;
        count++;
        printf("%2d. %-18s (mana: %d)  %s\n", count, abilityName(ab),
               abilityManaCost(ab), abilityDescription(ab));
    }

    if (count == 0)
    {
        printf("You have no class abilities yet.\n");
        return 0;
    }

    printf(" 0. Back\n");
    printf("Enter the number of your choice: ");

    int choice = readInt();

    if (choice < 1 || choice > count)
        return 0;

    int ab = list[choice - 1];
    int cost = abilityManaCost(ab);

    /* Bloodstone: allow HP cost for mana-based abilities */
    int useHP = 0;
    if (cost > 0 && player.mana < cost && relicBloodstoneActive())
    {
        int hpCost = cost * 3 / 2;   /* 1.5x in HP */
        if (player.health > hpCost)
        {
            printf("Not enough mana. Bloodstone activates: pay %d HP instead? (1=yes, 0=no): ", hpCost);
            if (readInt() == 1)
            {
                useHP = 1;
                cost = hpCost;
            }
            else
            {
                printf("Not enough mana!\n");
                return 0;
            }
        }
        else
        {
            printf("Not enough mana or HP for Bloodstone!\n");
            return 0;
        }
    }
    else if (cost > 0 && player.mana < cost)
    {
        printf("Not enough mana!\n");
        return 0;
    }
    else if (cost == 0 && abilityDiscipline(ab) >= DISC_BLOOD && abilityDiscipline(ab) <= DISC_PACT)
    {
        /* Warlock abilities with 0 mana cost - they cost HP */
        /* HP cost is already handled per-ability below */
    }

    /* Pay the cost */
    if (useHP)
    {
        player.health -= cost;
        if (player.health < 1) player.health = 1;
        printf("Bloodstone drains %d HP. Health: %d/%d\n", cost, player.health, player.maxHealth);
    }
    else if (cost > 0)
    {
        player.mana -= cost;
    }

    /* Execute the ability effect */
    printf("\nYou use %s!\n", abilityName(ab));

    switch (ab)
    {
    /* === KNIGHT / VANGUARD === */
    case AB_GUARD:
        *combatState |= CSTATE_GUARDING;
        player.defense += 5;
        printf("You raise your guard. The next hit will be halved, and you gain +5 defense.\n");
        return 1;

    case AB_SHIELD_BASH:
    {
        int dmg = player.defense / 2 + 10 + rand() % 8;
        dmg += relicOnDealDamage(dmg, 0);
        if (dmg < 1) dmg = 1;
        printf("You slam your shield into the enemy for %d damage!\n", dmg);
        /* Stun effect handled by combat.c checking gStunTurns */
        return 1;  /* combat.c will set gStunTurns = 1 */
    }

    case AB_IRON_FORTRESS:
        *combatState |= CSTATE_GUARDING;
        adv.ironFortressTurns = 3;
        printf("You become an iron fortress! All damage halved for 3 turns.\n");
        return 1;

    /* === KNIGHT / DUELIST === */
    case AB_RIPOSTE:
        *combatState |= CSTATE_COUNTER_READY;
        printf("You enter a defensive stance. The next hit will trigger a powerful counter!\n");
        return 1;

    case AB_PRECISION:
        adv.precisionActive = 1;
        printf("You focus your blade. Your next attack has +25%% critical chance and cannot miss.\n");
        return 1;

    case AB_EXECUTION:
    {
        /* Lethal strike - bonus to low HP enemies */
        int dmg = player.attack / 2 + 15 + rand() % 15;
        /* Triple damage if enemy below 30% HP */
        if (enemyHPPercent <= 30)
        {
            dmg *= 3;
            printf("EXECUTION! The enemy is weakened - devastating blow!\n");
        }
        dmg += relicOnDealDamage(dmg, 0);
        if (dmg < 1) dmg = 1;
        printf("You strike for %d damage!\n", dmg);
        return 1;
    }

    /* === KNIGHT / WARLORD === */
    case AB_WAR_CRY:
        *combatState |= CSTATE_WARCRY;
        adv.warcryTurns = 2;
        printf("Your war cry echoes! Attack +30%% for 2 turns!\n");
        return 1;

    case AB_COMMAND:
        adv.commandActive = 1;
        printf("You issue a command! The enemy hesitates and loses its next turn.\n");
        return 1;

    case AB_WARLORDS_MIGHT:
        adv.warlordsMightActive = 1;
        printf("You channel the Warlord's Might! Your next attack deals double damage and cannot be blocked!\n");
        return 1;

    /* === ARCANIST / PYROMANCER === */
    case AB_IGNITE:
    {
        int dmg = 15 + player.attack / 4 + rand() % 10;
        dmg = relicOnDealDamage(dmg, 1);
        printf("You set the enemy ablaze for %d damage! It will burn for 3 turns.\n", dmg);
        adv.burnTurns = 3;
        adv.burnDamage = 5 + player.level;
        return 1;
    }

    case AB_PYRO_FIREBALL:
    {
        int dmg = 35 + player.attack / 3 + rand() % 20;
        dmg = relicOnDealDamage(dmg, 1);
        if (player.skills[SK_FIRE]) dmg += 12;
        printf("A massive fireball crashes for %d damage!\n", dmg);
        return 1;
    }

    case AB_INFERNO:
    {
        int dmg = 80 + player.attack / 2 + rand() % 30;
        dmg = relicOnDealDamage(dmg, 1);
        if (player.skills[SK_FIRE]) dmg += 18;
        printf("INFERNO! The battlefield erupts for %d damage!\n", dmg);
        /* Also applies burn */
        adv.burnTurns = 3;
        adv.burnDamage = 8 + player.level;
        return 1;
    }

    /* === ARCANIST / CRYOMANCER === */
    case AB_FROSTBOLT:
    {
        int dmg = 18 + player.attack / 4 + rand() % 12;
        int freezeChance = 35 + relicFreezeBonus();
        printf("A frostbolt strikes for %d damage!\n", dmg);
        if (rand() % 100 < freezeChance)
        {
            printf("The enemy is FROZEN solid!\n");
            /* gFrozen is handled by combat.c */
            adv.cryoFreeze = 1;
        }
        return 1;
    }

    case AB_FREEZE_SPELL:
    {
        int dmg = 10 + player.attack / 6;
        printf("You cast Freeze! The enemy is frozen for 2 turns! (%d damage)\n", dmg);
        adv.cryoFreeze2 = 1;  /* 2 turn freeze - combat.c handles */
        return 1;
    }

    case AB_ABSOLUTE_ZERO:
    {
        int dmg;
        if (enemyIsFrozen)
        {
            dmg = 100 + player.attack / 2 + rand() % 40;
            printf("ABSOLUTE ZERO! The frozen enemy SHATTERS for %d damage!\n", dmg);
        }
        else
        {
            dmg = 50 + player.attack / 3 + rand() % 20;
            printf("Absolute Zero deals %d damage. Would be devastating if the enemy were frozen...\n", dmg);
        }
        dmg = relicOnDealDamage(dmg, 0);
        return 1;
    }

    /* === ARCANIST / STORMCALLER === */
    case AB_SPARK:
    {
        int dmg = 25 + player.attack / 3 + rand() % 10;
        printf("A spark of lightning ignores armour for %d damage!\n", dmg);
        return 1;
    }

    case AB_LIGHTNING_STORM:
    {
        int dmg = 20 + player.attack / 4 + rand() % 10;
        printf("A lightning storm begins! 3 bolts will strike over 3 turns. (%d damage each)\n", dmg);
        adv.stormTurns = 3;
        adv.stormDamage = dmg;
        return 1;
    }

    case AB_TEMPEST:
    {
        int dmg = 90 + player.attack / 2 + rand() % 30;
        printf("THE TEMPEST UNLEASHED! %d damage, ignoring all defence!\n", dmg);
        return 1;
    }

    /* === ROGUE / ASSASSIN === */
    case AB_BLEED:
    {
        int dmg = player.attack / 3 + 8 + rand() % 8;
        printf("You slash to inflict bleeding for %d damage! The enemy will bleed for 3 turns.\n", dmg);
        adv.bleedTurns = 3;
        adv.bleedDamage = 5 + player.level;
        return 1;
    }

    case AB_BACKSTAB:
    {
        int dmg = player.attack / 2 + 10 + rand() % 12;
        if (playerActingFirst)
        {
            dmg *= 2;
            printf("You strike before the enemy can react! BACKSTAB for %d damage!\n", dmg);
        }
        else
        {
            printf("Backstab deals %d damage. (Would be double if you acted first!)\n", dmg);
        }
        dmg += relicOnDealDamage(dmg, 0);
        return 1;
    }

    case AB_ASSASSIN_EXECUTE:
    {
        /* Execute: instant kill below 25%, heavy damage otherwise */
        if (enemyHPPercent <= 25)
        {
            printf("ASSASSIN'S EXECUTE! A lethal blow - the enemy is instantly defeated!\n");
            adv.instantKill = 1;
        }
        else
        {
            int dmg = player.attack + 20 + rand() % 20;
            printf("Execute strikes for %d damage. (Would be lethal if the enemy were below 25%% HP!)\n", dmg);
        }
        return 1;
    }

    /* === ROGUE / TRICKSTER === */
    case AB_FEINT:
        *combatState |= CSTATE_FEINTING;
        printf("You feint! The enemy's next attack will miss.\n");
        return 1;

    case AB_DISORIENT:
        printf("You disorient the enemy! 50%% chance it hurts itself next turn.\n");
        adv.disorientActive = 1;
        return 1;

    case AB_SHADOW_STEP:
        *combatState |= CSTATE_SHADOW_VEIL;
        adv.shadowStepTurns = 2;
        printf("You shadow step! You dodge all attacks for 2 turns.\n");
        return 1;

    /* === ROGUE / POISONER === */
    case AB_ENVENOM:
    {
        int dmg = player.attack / 4 + 5 + rand() % 6;
        printf("You envenom your blade! +%d damage now, and poison for 3 turns.\n", dmg);
        adv.poisonTurns = 3;
        adv.poisonDamage = 4 + player.level;
        return 1;
    }

    case AB_TOXIC_CLOUD:
    {
        int dmg = 20 + player.level * 2 + rand() % 12;
        printf("A toxic cloud engulfs the enemy for %d damage! Poison continues for 3 turns.\n", dmg);
        adv.poisonTurns = 3;
        adv.poisonDamage = 6 + player.level;
        return 1;
    }

    case AB_PLAGUE:
    {
        int dmg = 35 + player.level * 3 + rand() % 15;
        printf("PLEGUE! %d damage now, and virulent poison for 3 turns!\n", dmg);
        adv.poisonTurns = 3;
        adv.poisonDamage = 10 + player.level * 2;
        return 1;
    }

    /* === RANGER / ARCHER === */
    case AB_AIM_SHOT:
    {
        int dmg = player.attack / 2 + 12 + rand() % 12;
        int critChance = 50 + player.critChance;
        if (critChance > 100) critChance = 100;
        int crit = (rand() % 100 < critChance);
        if (crit) dmg *= 2;
        printf("A carefully aimed shot for %d damage!%s\n", dmg, crit ? " CRITICAL HIT!" : "");
        return 1;
    }

    case AB_PIERCING_ARROW:
    {
        int dmg = player.attack / 2 + 10 + rand() % 10;
        printf("A piercing arrow ignores armour for %d damage!\n", dmg);
        return 1;
    }

    case AB_RAIN_ARROWS:
    {
        int total = 0;
        printf("A rain of arrows!\n");
        for (int i = 0; i < 3; i++)
        {
            int hit = player.attack / 3 + 8 + rand() % 8;
            total += hit;
            printf("  Arrow %d: %d damage\n", i + 1, hit);
        }
        printf("Total: %d damage!\n", total);
        return 1;
    }

    /* === RANGER / HUNTER === */
    case AB_MARK_PREY:
        *combatState |= CSTATE_MARKED;
        adv.markTurns = 3;
        printf("You mark your prey! All attacks deal +30%% damage for 3 turns.\n");
        return 1;

    case AB_HUNTERS_FOCUS:
        adv.huntersFocusTurns = 2;
        printf("Hunter's Focus! +50%% critical chance and +30%% damage for 2 turns.\n");
        return 1;

    case AB_DEVASTATING_SHOT:
    {
        int dmg = player.attack + 15 + rand() % 15;
        int isMarked = (*combatState & CSTATE_MARKED) != 0;
        int crit = isMarked || (rand() % 100 < player.critChance);
        if (crit)
        {
            dmg *= 2;
            printf("DEVASTATING SHOT! %s Critical hit for %d damage!\n",
                   isMarked ? "Marked target -" : "", dmg);
        }
        else
        {
            printf("Devastating shot for %d damage!\n", dmg);
        }
        return 1;
    }

    /* === RANGER / TRAPPER === */
    case AB_SET_TRAP:
        *combatState |= CSTATE_TRAP;
        printf("You set a trap! When the enemy attacks, it will trigger a counter-hit.\n");
        return 1;

    case AB_SNARE:
        printf("You snare the enemy! It loses its next 2 turns.\n");
        adv.snareTurns = 2;
        return 1;

    case AB_GRAND_TRAP:
    {
        int dmg = 40 + player.level * 3 + rand() % 20;
        printf("GRAND TRAP triggers! %d damage and the enemy is stunned for 2 turns!\n", dmg);
        adv.grandTrapStun = 2;
        return 1;
    }

    /* === WARLOCK / BLOOD === */
    case AB_SACRIFICE:
    {
        int hpCost = player.maxHealth * 15 / 100;
        if (hpCost < 10) hpCost = 10;
        player.health -= hpCost;
        if (player.health < 1) player.health = 1;
        adv.sacrificeActive = 1;
        printf("You sacrifice %d HP. Your next attack deals +50%% damage! HP: %d/%d\n",
               hpCost, player.health, player.maxHealth);
        return 1;
    }

    case AB_BLOOD_MAGIC:
    {
        int hpCost = player.maxHealth * 20 / 100;
        if (hpCost < 15) hpCost = 15;
        player.health -= hpCost;
        if (player.health < 1) player.health = 1;
        int dmg = 40 + player.attack / 2 + rand() % 15;
        int heal = dmg / 2;
        printf("BLOOD MAGIC! You sacrifice %d HP, deal %d damage, and heal for %d!\n",
               hpCost, dmg, heal);
        healPlayer(&player, heal);
        return 1;
    }

    case AB_BLOOD_PACT:
    {
        int hpCost = player.maxHealth * 30 / 100;
        if (hpCost < 20) hpCost = 20;
        player.health -= hpCost;
        if (player.health < 1) player.health = 1;
        adv.bloodPactTurns = 3;
        printf("BLOOD PACT! You sacrifice %d HP. Your next 3 attacks deal double damage and heal you! HP: %d/%d\n",
               hpCost, player.health, player.maxHealth);
        return 1;
    }

    /* === WARLOCK / SHADOW === */
    case AB_SHADOW_BOLT:
    {
        int dmg = 20 + player.attack / 3 + rand() % 10;
        printf("A shadow bolt for %d damage - cannot be blocked!\n", dmg);
        return 1;
    }

    case AB_SHADOW_VEIL:
        *combatState |= CSTATE_SHADOW_VEIL;
        adv.shadowStepTurns = 2;
        printf("Shadow veil! 50%% evasion for 2 turns.\n");
        return 1;

    case AB_SOUL_DRAIN:
    {
        int dmg = 35 + player.attack / 2 + rand() % 15;
        printf("Soul Drain! %d damage and you heal for the full amount!\n", dmg);
        healPlayer(&player, dmg);
        return 1;
    }

    /* === WARLOCK / PACT === */
    case AB_DARK_PACT:
    {
        int hpCost = player.maxHealth * 10 / 100;
        if (hpCost < 8) hpCost = 8;
        player.health -= hpCost;
        if (player.health < 1) player.health = 1;
        int manaGain = player.maxMana * 40 / 100;
        restoreMana(&player, manaGain);
        printf("Dark Pact! Sacrifice %d HP, regain %d mana. HP: %d/%d  Mana: %d/%d\n",
               hpCost, manaGain, player.health, player.maxHealth, player.mana, player.maxMana);
        return 1;
    }

    case AB_FORBIDDEN_POWER:
    {
        adv.forbiddenPowerTurns = 2;
        printf("FORBIDDEN POWER! Attack doubled for 2 turns, but you take 10%% max HP damage per turn.\n");
        return 1;
    }

    case AB_PACT_SUMMON:
    {
        int dmg = player.level * 3;
        adv.pactSummonTurns = 3;
        adv.pactSummonDamage = dmg;
        printf("You summon a dark ally! It will deal %d damage per turn for 3 turns.\n", dmg);
        return 1;
    }

    /* === GAMBLER / FORTUNE === */
    case AB_LUCKY_STRIKE:
    {
        int dmg = player.attack / 2 + 10 + rand() % 10;
        if (rand() % 100 < 50)
        {
            dmg *= 2;
            printf("LUCKY STRIKE! Double damage for %d!\n", dmg);
        }
        else
        {
            printf("Lucky strike deals %d damage. Not your luckiest roll.\n", dmg);
        }
        return 1;
    }

    case AB_WILD_CARD:
    {
        int effect = rand() % 4;
        switch (effect)
        {
        case 0:
            healPlayer(&player, 30 + player.level * 3);
            printf("WILD CARD: Healing! You recover %d HP!\n", 30 + player.level * 3);
            break;
        case 1:
        {
            int dmg = 40 + rand() % 30;
            printf("WILD CARD: Damage! A blast for %d!\n", dmg);
            break;
        }
        case 2:
            adv.wildCardBuff = 1;
            printf("WILD CARD: Buff! Your next attack deals double damage!\n");
            break;
        default:
            printf("WILD CARD: ...Nothing happens. The card was blank.\n");
            break;
        }
        return 1;
    }

    case AB_JACKPOT:
    {
        int hits = 0;
        int totalDmg = 0;
        for (int i = 0; i < 3; i++)
        {
            if (rand() % 100 < 33)
            {
                int dmg = 30 + player.level * 2 + rand() % 20;
                totalDmg += dmg;
                hits++;
                printf("  JACKPOT hit %d: %d damage!\n", i + 1, dmg);
            }
            else
                printf("  Jackpot roll %d: miss.\n", i + 1);
        }
        if (hits == 3)
            printf("ALL THREE HIT! JACKPOT! Total: %d damage!\n", totalDmg);
        else if (hits > 0)
            printf("Total jackpot damage: %d\n", totalDmg);
        else
            printf("The jackpot eludes you this time.\n");
        return 1;
    }

    /* === GAMBLER / RISK === */
    case AB_ALL_IN:
    {
        int hpCost = player.maxHealth * 25 / 100;
        if (hpCost < 15) hpCost = 15;
        int chance = 55 + classGamblingBonus(adv.playerClass) + relicGamblingBonus();
        if (rand() % 100 < chance)
        {
            int dmg = (player.attack / 2 + 10 + rand() % 10) * 3;
            printf("ALL IN - YOU WIN! Triple damage for %d!\n", dmg);
        }
        else
        {
            player.health -= hpCost;
            if (player.health < 1) player.health = 1;
            printf("ALL IN - YOU LOSE! You lose %d HP. HP: %d/%d\n", hpCost, player.health, player.maxHealth);
        }
        return 1;
    }

    case AB_DOUBLE_DOWN:
    {
        adv.doubleDownActive = 1;
        printf("Double Down! Attack doubled this turn. If you miss, you take 20%% max HP damage.\n");
        return 1;
    }

    case AB_HIGH_ROLLER:
        adv.highRollerTurns = 3;
        printf("High Roller! For 3 turns, every attack has 40%% chance to deal double damage.\n");
        return 1;

    /* === GAMBLER / DEALER === */
    case AB_CARD_SHARP:
    {
        int card = rand() % 4;
        const char *suits[] = { "Hearts", "Diamonds", "Clubs", "Spades" };
        printf("You draw: %s! ", suits[card]);
        switch (card)
        {
        case 0: /* Hearts */
            healPlayer(&player, 20 + player.level * 2);
            printf("Heal for %d!\n", 20 + player.level * 2);
            break;
        case 1: /* Diamonds */
            player.gold += 15 + player.level * 3;
            printf("+%d gold!\n", 15 + player.level * 3);
            break;
        case 2: /* Clubs */
            adv.cardSharpBuff = 1;
            printf("+30%% damage on next attack!\n");
            break;
        case 3: /* Spades */
        {
            int dmg = 25 + player.level * 2 + rand() % 10;
            printf("Enemy takes %d damage!\n", dmg);
            break;
        }
        }
        return 1;
    }

    case AB_STACKED_DECK:
        adv.stackedDeckTurns = 3;
        printf("Stacked Deck! For 3 turns, all gambling abilities have improved outcomes.\n");
        return 1;

    case AB_DEALERS_CHOICE:
    {
        printf("Dealer's Choice! Five random effects:\n");
        for (int i = 0; i < 5; i++)
        {
            int fx = rand() % 6;
            switch (fx)
            {
            case 0:
                printf("  %d. Heal %d\n", i + 1, 15 + rand() % 15);
                healPlayer(&player, 15 + rand() % 15);
                break;
            case 1:
            {
                int dmg = 15 + rand() % 25;
                printf("  %d. Blast for %d\n", i + 1, dmg);
                break;
            }
            case 2:
                printf("  %d. +%d gold\n", i + 1, 10 + rand() % 20);
                player.gold += 10 + rand() % 20;
                break;
            case 3:
                printf("  %d. Restore %d mana\n", i + 1, 10 + rand() % 15);
                restoreMana(&player, 10 + rand() % 15);
                break;
            case 4:
                printf("  %d. Poison! (3 turns)\n", i + 1);
                adv.poisonTurns += 3;
                adv.poisonDamage += 3;
                break;
            default:
                printf("  %d. ...nothing.\n", i + 1);
                break;
            }
        }
        return 1;
    }

    default:
        printf("Ability not yet implemented.\n");
        return 0;
    }
}
