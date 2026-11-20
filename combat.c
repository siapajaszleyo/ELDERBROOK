#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "combat.h"
#include "input.h"
#include "adventure.h"
#include "items.h"
#include "skills.h"
#include "class.h"
#include "monster.h"
#include "dungeonmod.h"
#include "weapons.h"
#include "path.h"
#include "balance.h"
#include "ascii.h"
#include "ui.h"

#define RUN_SUCCESS_PERCENT    70   /* chance that running away works       */
#define MAX_DEFENSE_REDUCTION  20   /* defense can never cut more than this */

/* State that lasts for one fight only */
static int gShield;                 /* 1 = the next hit is halved          */
static int gBrace;
static int gGuard;
static int gStamina;
static int gHasteExtra;
static int gSkillCooldown;

typedef enum { ST_POISON, ST_BURN, ST_BLEED, ST_STUN, ST_SLOW,
               ST_WEAKEN, ST_VULNERABLE, ST_REGEN, ST_SHIELD, ST_HASTE,
               ST_COUNT } StatusId;
typedef struct { int turns, magnitude, stacks; } StatusState;
typedef struct { const char *name; int stacking; } StatusRule;
static const StatusRule statusRules[ST_COUNT] = {
    {"Poison",1}, {"Burn",0}, {"Bleed",1}, {"Stun",0}, {"Slow",0},
    {"Weaken",0}, {"Vulnerable",0}, {"Regen",0}, {"Shield",1}, {"Haste",0}
};
static StatusState enemyStatus[ST_COUNT];
static StatusState playerStatus[ST_COUNT];

static void applyStatus(StatusState *list, StatusId id, int turns, int magnitude)
{
    StatusState *s = &list[id];
    if (statusRules[id].stacking && s->turns > 0)
    {
        s->stacks++;
        if (s->stacks > 3) s->stacks = 3;
        s->magnitude += magnitude;
    }
    else
    {
        s->stacks = 1;
        s->magnitude = magnitude;
    }
    if (s->magnitude > 99) s->magnitude = 99;
    if (turns > s->turns) s->turns = turns;
    printf("%s (%d turns) applied.\n", statusRules[id].name, s->turns);
}
static int gFrozen;                 /* 1 = the enemy loses its next turn   */
static int gIronWillUsed;           /* Iron Will works once per fight      */
static int gStunTurns;               /* Enemy stun turns remaining          */
static int gSnareTurns;              /* Enemy snare turns remaining          */
static int gCombatState;             /* CSTATE_* bitmask for this fight     */
static int gDmgThisTurn;             /* Damage dealt by class ability this turn */


static int readCombatChoice(void)
{
    printf("Enter your choice: ");
    return readInt();           /* -1 if the player typed something silly */
}


void initializeEnemy(Enemy *enemy,
                     const char *name,
                     int level,
                     int health,
                     int attack,
                     int defense,
                     int xpReward,
                     int goldReward,
                     EnemySpecial special)
{
    strncpy(enemy->name, name, sizeof(enemy->name) - 1);
    enemy->name[sizeof(enemy->name) - 1] = '\0';

    enemy->level = level;

    enemy->health = health;
    enemy->maxHealth = health;

    enemy->attack = attack;
    enemy->defense = defense;

    enemy->xpReward = xpReward;
    enemy->goldReward = goldReward;

    enemy->special = special;
    enemy->special2 = SPECIAL_NONE;
    enemy->type = -1;
    enemy->dropMaterial = -1;
    enemy->dropChance = 0;
    enemy->archetype = ENEMY_BRUTE;
    enemy->phase = 0;
}


static int hasSpecial(const Enemy *enemy, EnemySpecial special)
{
    return enemy->special == special || enemy->special2 == special;
}

static void printEnemyTag(const Enemy *enemy)
{
    printf("%s (Lv %d %s)", enemy->name, enemy->level, rankName(enemy->level));
}

static const char *specialText(EnemySpecial special)
{
    switch (special)
    {
        case SPECIAL_REGEN:  return "It slowly regenerates.";
        case SPECIAL_DOUBLE: return "It is fast and may strike twice.";
        case SPECIAL_RAGE:   return "It grows furious when badly hurt.";
        case SPECIAL_DRAIN:  return "Its touch drains your mana.";
        case SPECIAL_BURN:   return "It can breathe fire.";
        default:             return NULL;
    }
}


/* ---------------------------------------------------------
   DEALING DAMAGE TO THE ENEMY
   --------------------------------------------------------- */

static void dealDamage(Enemy *enemy, int damage)
{
    if (enemyStatus[ST_VULNERABLE].turns > 0)
        damage += damage * enemyStatus[ST_VULNERABLE].magnitude / 100;
    /* A single blow that would have killed a healthy enemy outright
       (the "Overkill" achievement) */
    if (enemy->health == enemy->maxHealth && damage >= enemy->maxHealth)
        adv.stats[ST_BIG_HIT] = 1;

    enemy->health -= damage;

    if (enemy->health < 0)
        enemy->health = 0;
}

static void tickEnemyStatus(Enemy *enemy)
{
    for (int id = ST_POISON; id <= ST_BLEED; id++)
    {
        StatusState *s = &enemyStatus[id];
        if (s->turns <= 0) continue;
        int damage = id == ST_BLEED ? enemy->maxHealth * s->magnitude / 100 : s->magnitude;
        if (damage < 1) damage = 1;
        dealDamage(enemy, damage);
        printf("%s takes %d %s damage.\n", enemy->name, damage, statusRules[id].name);
        s->turns--;
    }
    if (enemyStatus[ST_SLOW].turns > 0) enemyStatus[ST_SLOW].turns--;
    if (enemyStatus[ST_WEAKEN].turns > 0) enemyStatus[ST_WEAKEN].turns--;
    if (enemyStatus[ST_VULNERABLE].turns > 0) enemyStatus[ST_VULNERABLE].turns--;
}

static void tickPlayerStatus(Player *player)
{
    for (int id = ST_POISON; id <= ST_BLEED; id++)
    {
        StatusState *s = &playerStatus[id];
        if (s->turns <= 0) continue;
        int damage = id == ST_BLEED ? player->maxHealth * s->magnitude / 100 : s->magnitude;
        if (damage < 1) damage = 1;
        damagePlayer(player, damage);
        printf("You take %d %s damage.\n", damage, statusRules[id].name);
        s->turns--;
    }
    if (playerStatus[ST_REGEN].turns > 0)
    {
        int amount = playerStatus[ST_REGEN].magnitude;
        healPlayer(player, amount);
        playerStatus[ST_REGEN].turns--;
        printf("Regen restores %d HP.\n", amount);
    }
    if (playerStatus[ST_SLOW].turns > 0) playerStatus[ST_SLOW].turns--;
    if (playerStatus[ST_WEAKEN].turns > 0) playerStatus[ST_WEAKEN].turns--;
    if (playerStatus[ST_HASTE].turns > 0) playerStatus[ST_HASTE].turns--;
}

static int weaponBonus(int fx)
{
    switch (fx)
    {
        case WFX_FLAME:   return 6;
        case WFX_DRAGON:  return 10;
        case WFX_LEGEND:  return 8;
        case WFX_PLUNDER: return 4;
        case WFX_DUEL:    return 0;  /* counter-attack handled in enemyAttack */
        case WFX_EXECUTE: return 0; /* bonus to low-HP handled in playerAttack */
        case WFX_SPELL:   return 0; /* spell amp handled in fireball */
        case WFX_BLOOD:   return 0; /* lifesteal handled in playerAttack */
        case WFX_GAMBLE:  return 0; /* crit variance handled in rollSwordDamage */
        default:          return 0;
    }
}

/* The raw power of one blow BEFORE a critical hit: Attack Power, a roll,
   the weapon's flat effect, the Berserker skill, and the enemy's armour.
   Weapon movesets build on this so every type shares one honest base. */
static int rawWeaponDamage(const Player *player, const Enemy *enemy)
{
    /* Damage model: max(1, ATK/2 + 5 + d11 + weapon bonus - armour/2),
       then critical hits double the result. Incoming damage subtracts
       one third of defence (capped by MAX_DEFENSE_REDUCTION). */
    int armor = enemy->defense;
    if (enemyStatus[ST_BURN].turns > 0)
        armor -= enemyStatus[ST_BURN].magnitude;
    if (armor < 0) armor = 0;
    int attack = player->attack;
    if (playerStatus[ST_WEAKEN].turns > 0)
        attack -= playerStatus[ST_WEAKEN].magnitude;
    if (attack < 1) attack = 1;
    int damage = attack / 2 + 5 + rand() % 11 - armor / 2;

    damage += weaponBonus(player->weaponFx);

    if (player->skills[SK_BERSERK] && player->health * 100 < player->maxHealth * 40)
        damage = damage * 3 / 2;

    if (damage < 1)
        damage = 1;

    return damage;
}

/* Roll whether this blow crits. */
static int rollCrit(const Player *player)
{
    int chance = player->critChance + (player->skills[SK_CRIT] ? 15 : 0);
    if (chance > 100) chance = 100;
    if (chance < 0) chance = 0;
    return rand() % 100 < chance;
}

/* A sword blow: Attack Power, a roll, the weapon's effect, the Berserker
   skill, armour on the enemy, and a chance of a critical hit. */
static int rollSwordDamage(const Player *player, const Enemy *enemy, int *crit)
{
    int damage = rawWeaponDamage(player, enemy);

    *crit = rollCrit(player);

    if (*crit)
        damage *= 2;

    return damage;
}

/* Effects that trigger when a weapon hits */
static void weaponHitEffects(Player *player, Enemy *enemy)
{
    if (enemy->health <= 0)
        return;

    if (player->weaponFx == WFX_FROST && !gFrozen && rand() % 100 < 20)
    {
        gFrozen = 1;
        printf("%s is frozen solid!\n", enemy->name);
    }

    if (player->weaponFx == WFX_LEGEND)
        healPlayer(player, 3);

    if (player->weaponFx == WFX_PLUNDER)
        player->gold += 5;

    /* Bloodfang lifesteal */
    if (player->weaponFx == WFX_BLOOD)
    {
        int ls = 3 + rand() % 4;
        healPlayer(player, ls);
    }

    /* Gambler's Blade: fluctuate crit each hit */
    if (player->weaponFx == WFX_GAMBLE)
        player->critChance = 5 + rand() % 31;
}

/* Apply every active combat buff/debuff multiplier to a blow's damage,
   consuming one-shot buffs as it goes. Shared by the plain attack and by
   all weapon movesets so buffs work no matter which move you pick. */
static int applyCombatBuffs(Player *player, Enemy *enemy, int damage, int *crit)
{
    (void)enemy;

    /* War Cry buff: +30% damage */
    if (adv.warcryTurns > 0)
        damage = damage * 13 / 10;

    /* Hunter's Focus: +30% damage */
    if (adv.huntersFocusTurns > 0)
        damage = damage * 13 / 10;

    /* Mark Prey: +30% damage */
    if (adv.markTurns > 0)
        damage = damage * 13 / 10;

    /* Sacrifice buff: +50% damage */
    if (adv.sacrificeActive)
    {
        damage = damage * 3 / 2;
        adv.sacrificeActive = 0;
    }

    /* Blood Pact: double damage and lifesteal */
    if (adv.bloodPactTurns > 0)
    {
        damage *= 2;
        healPlayer(player, damage / 2);
        printf("Blood Pact surges! You heal for %d.\n", damage / 2);
    }

    /* Warlord's Might: double damage */
    if (adv.warlordsMightActive)
    {
        damage *= 2;
        adv.warlordsMightActive = 0;
    }

    /* Wild Card buff: double damage */
    if (adv.wildCardBuff)
    {
        damage *= 2;
        adv.wildCardBuff = 0;
    }

    /* Card Sharp buff: +30% damage */
    if (adv.cardSharpBuff)
    {
        damage = damage * 13 / 10;
        adv.cardSharpBuff = 0;
    }

    /* Double Down: double attack, but risk */
    if (adv.doubleDownActive)
    {
        damage *= 2;
        adv.doubleDownActive = 0;
    }

    /* High Roller: 40% chance for double damage */
    if (adv.highRollerTurns > 0 && rand() % 100 < 40)
    {
        damage *= 2;
        printf("High Roller! Double damage!\n");
    }

    /* Precision: guaranteed hit already handled; try to force a crit */
    if (adv.precisionActive)
    {
        adv.precisionActive = 0;
        if (crit && !*crit && rand() % 100 < 25)
        {
            *crit = 1;
            damage *= 2;
        }
    }

    if (damage < 1)
        damage = 1;

    return damage;
}

/* Land one weapon blow of a given pre-buff damage, applying the
   Executioner's Axe low-HP bonus, all combat buffs, the hit print-out
   and (optionally) on-hit weapon effects. */
static void doWeaponHit(Player *player, Enemy *enemy, int damage, int crit,
                        const char *label, int applyEffects)
{
    /* Executioner's Axe: bonus damage to low-HP enemies */
    if (player->weaponFx == WFX_EXECUTE && enemy->health * 100 < enemy->maxHealth * 30)
        damage += damage / 2;

    damage = applyCombatBuffs(player, enemy, damage, &crit);

    dealDamage(enemy, damage);

    printf("\n%s for %d damage!%s\n", label, damage, crit ? "  CRITICAL HIT!" : "");
    printf("%s HP: %d/%d\n", enemy->name, enemy->health, enemy->maxHealth);

    if (applyEffects)
        weaponHitEffects(player, enemy);
}

void playerAttack(Player *player, Enemy *enemy)
{
    int crit;
    int damage = rollSwordDamage(player, enemy, &crit);

    char label[96];
    snprintf(label, sizeof label, "You attack %s", enemy->name);
    doWeaponHit(player, enemy, damage, crit, label, 1);
}

/* =========================================================
   WEAPON MOVESETS
   The "Attack" option opens the moveset for the equipped weapon's
   TYPE (see weapons.c). Returns 1 if a move was used (the turn is
   spent), 0 if the player backed out.
   ========================================================= */
static int weaponAttackMenu(Player *player, Enemy *enemy)
{
    WeaponType wt = weaponTypeOf(adv.weapon);

    printf("\n--- %s: %s ---\n", weaponTypeName(wt), weaponTypeTagline(wt));
    for (int m = 0; m < WEAPON_MOVES; m++)
        printf("%d. %s — %s\n", m + 1, weaponMoveName(wt, m), weaponMoveDesc(wt, m));
    printf("0. Back\n");
    printf("Choose your strike: ");

    int choice = readCombatChoice();
    if (choice < 1 || choice > WEAPON_MOVES)
        return 0;

    int move = choice - 1;
    char label[96];
    int base = rawWeaponDamage(player, enemy);
    int crit = rollCrit(player);

    switch (wt)
    {
    case WT_SWORD:
        if (move == 0)
        {
            int d = base; if (crit) d *= 2;
            snprintf(label, sizeof label, "You slash %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        else
        {
            int d = base * 4 / 5; if (crit) d *= 2;  /* weaker hit */
            snprintf(label, sizeof label, "You batter %s's guard", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
            if (enemy->defense > 0)
            {
                int drop = 2 + enemy->defense / 4;
                enemy->defense -= drop;
                if (enemy->defense < 0) enemy->defense = 0;
                printf("Its armour is cracked! (defence -%d for this fight)\n", drop);
            }
        }
        break;

    case WT_DAGGER:
        if (move == 0)
        {
            if (!crit && rollCrit(player)) crit = 1;   /* extra crit roll */
            int d = base * 7 / 10; if (crit) d *= 2;
            snprintf(label, sizeof label, "You stab %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        else
        {
            printf("\nYou unleash a flurry of strikes!\n");
            for (int h = 0; h < 2 && enemy->health > 0; h++)
            {
                int c = rollCrit(player);
                int d = base * 6 / 10; if (c) d *= 2;
                snprintf(label, sizeof label, "Hit %d on %s", h + 1, enemy->name);
                doWeaponHit(player, enemy, d, c, label, h == 1);
            }
        }
        break;

    case WT_STAFF:
        if (move == 0)
        {
            int d = base * 6 / 10 + player->mana / 3; if (crit) d *= 2;
            snprintf(label, sizeof label, "Your staff blasts %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        else
        {
            if (player->mana < 10)
            {
                printf("Not enough mana to channel!\n");
                return 0;
            }
            player->mana -= 10;
            /* pierces defence: add back half the enemy's armour reduction */
            int d = base + enemy->defense / 2 + 6; if (crit) d *= 2;
            snprintf(label, sizeof label, "Channelled power tears into %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        break;

    case WT_AXE:
        if (move == 0)
        {
            int d = base * 11 / 10 + rand() % 7 - 3; if (crit) d *= 2;
            snprintf(label, sizeof label, "You chop into %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        else
        {
            if (rand() % 100 < 25)
            {
                printf("\nYou wind up a reckless cleave… and %s steps aside! You miss!\n",
                       enemy->name);
            }
            else
            {
                int d = (base + enemy->defense / 2) * 16 / 10; if (crit) d *= 2;
                snprintf(label, sizeof label, "Your cleave crashes through %s", enemy->name);
                doWeaponHit(player, enemy, d, crit, label, 1);
            }
        }
        break;

    case WT_BOW:
        if (move == 0)
        {
            int d = base; if (crit) d *= 2;
            snprintf(label, sizeof label, "Your arrow strikes %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
            /* a safe ranged shot — no counter window (handled by not setting any) */
        }
        else
        {
            if (rand() % 100 < 15)
            {
                printf("\nYou take careful aim… but the shot goes wide!\n");
            }
            else
            {
                if (!crit && rollCrit(player)) crit = 1;
                int d = base * 14 / 10; if (crit) d *= 2;
                snprintf(label, sizeof label, "Your aimed shot pierces %s", enemy->name);
                doWeaponHit(player, enemy, d, crit, label, 1);
            }
        }
        break;

    case WT_BLADE:
    default:
        if (move == 0)
        {
            int d = base; if (crit) d *= 2;
            snprintf(label, sizeof label, "You cut %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 1);
        }
        else
        {
            int d = base * 13 / 10; if (crit) d *= 2;
            snprintf(label, sizeof label, "Elemental burst engulfs %s", enemy->name);
            doWeaponHit(player, enemy, d, crit, label, 0);
            /* guarantee the blade's elemental effect regardless of odds */
            switch (player->weaponFx)
            {
            case WFX_FROST:
                if (enemy->health > 0 && !gFrozen)
                {
                    gFrozen = 1;
                    printf("%s is frozen solid!\n", enemy->name);
                }
                break;
            case WFX_FLAME:
            case WFX_DRAGON:
                if (enemy->health > 0)
                {
                    adv.burnTurns = 3;
                    adv.burnDamage = 6 + player->level;
                    printf("%s is set ablaze! (burning)\n", enemy->name);
                }
                break;
            case WFX_LEGEND:
                healPlayer(player, 6);
                break;
            default:
                weaponHitEffects(player, enemy);
                break;
            }
        }
        break;
    }

    return 1;
}


/* ---------------------------------------------------------
   THE ENEMY'S TURN
   --------------------------------------------------------- */

static void hitPlayer(Player *player, const Enemy *enemy,
                      int damage, const char *how)
{
    if (damage < 1)
        damage = 1;

    if (playerStatus[ST_VULNERABLE].turns > 0)
        damage += damage * playerStatus[ST_VULNERABLE].magnitude / 100;

    if (playerStatus[ST_SHIELD].turns > 0)
    {
        int absorb = playerStatus[ST_SHIELD].magnitude;
        if (absorb > damage) absorb = damage;
        damage -= absorb;
        playerStatus[ST_SHIELD].turns = 0;
        printf("Your status shield absorbs %d damage.\n", absorb);
    }

    /* Iron Will: once per fight, survive a blow that would kill you */
    if (damage >= player->health && player->skills[SK_IRONWILL] && !gIronWillUsed)
    {
        gIronWillUsed = 1;
        damage = player->health - 1;
        printf("\nIRON WILL! You refuse to fall and cling to 1 health!\n");
    }

    damagePlayer(player, damage);

    printf("\n%s %s for %d damage!\n", enemy->name, how, damage);
    printf("Your HP: %d/%d\n", player->health, player->maxHealth);
}

void enemyAttack(Player *player, Enemy *enemy)
{
    if (enemy->phase == 0 && enemy->health * 2 <= enemy->maxHealth)
    {
        enemy->phase = 1;
        enemy->attack += enemy->attack / 4;
        printf("\n%s enters a second phase and grows more dangerous!\n", enemy->name);
    }
    if (enemy->phase == 1 && enemy->health * 10 <= enemy->maxHealth * 3)
    {
        enemy->phase = 2;
        enemy->attack += enemy->attack / 5;
        printf("%s enrages at low health!\n", enemy->name);
    }
    if (enemyStatus[ST_STUN].turns > 0)
    {
        enemyStatus[ST_STUN].turns--;
        printf("%s is stunned and loses its turn!\n", enemy->name);
        return;
    }
    /* --- Class evasion checks --- */
    if (gStunTurns > 0)
    {
        gStunTurns--;
        printf("\n%s is stunned and cannot act this turn!\n", enemy->name);
        return;
    }

    if (gSnareTurns > 0 || adv.snareTurns > 0)
    {
        if (gSnareTurns > 0) gSnareTurns--;
        if (adv.snareTurns > 0) adv.snareTurns--;
        printf("\n%s is snared and cannot act this turn!\n", enemy->name);
        return;
    }

    if (gFrozen)
    {
        gFrozen = 0;
        printf("\n%s is frozen and cannot act this turn!\n", enemy->name);
        return;
    }

    /* Shadow Step / Shadow Veil: dodge all attacks */
    if (adv.shadowStepTurns > 0)
    {
        adv.shadowStepTurns--;
        printf("\nYou vanish into shadow! %s's attack passes through nothing!\n", enemy->name);
        return;
    }

    /* Feint: enemy's next attack misses */
    if (gCombatState & CSTATE_FEINTING)
    {
        gCombatState &= ~CSTATE_FEINTING;
        printf("\nYour feint works! %s's attack goes wide!\n", enemy->name);
        return;
    }

    /* Class evasion bonus (Rogue +15%, Ranger +5%) */
    if (player->evasion > 0 && rand() % 100 < player->evasion)
    {
        printf("\nYou dodge %s's attack with practiced ease!\n", enemy->name);
        /* Duelist Rapier counter-attack */
        if (player->weaponFx == WFX_DUEL)
        {
            int crit;
            int back = rollSwordDamage(player, enemy, &crit) / 2;
            if (back < 1) back = 1;
            dealDamage(enemy, back);
            printf("Duelist counter-attack! You strike back for %d damage! (%s HP: %d/%d)\n",
                   back, enemy->name, enemy->health, enemy->maxHealth);
        }
        return;
    }

    if (player->armorFx == AFX_SHADOW && rand() % 100 < 15)
    {
        printf("\nYou slip into shadow and dodge %s's attack!\n", enemy->name);
        return;
    }

    /* Disorient: 50% chance enemy hurts itself */
    if (adv.disorientActive)
    {
        adv.disorientActive = 0;
        if (rand() % 100 < 50)
        {
            int selfDmg = enemy->attack / 2 + 5;
            dealDamage(enemy, selfDmg);
            printf("\n%s is disoriented and hurts itself for %d damage! (%s HP: %d/%d)\n",
                   enemy->name, selfDmg, enemy->name, enemy->health, enemy->maxHealth);
            return;
        }
    }

    /* Command: enemy loses its turn */
    if (adv.commandActive)
    {
        adv.commandActive = 0;
        printf("\nYour command forces %s to hesitate! It loses its turn!\n", enemy->name);
        return;
    }

    /* Trap: if set, trigger counter-hit when enemy attacks */
    if (gCombatState & CSTATE_TRAP)
    {
        gCombatState &= ~CSTATE_TRAP;
        int trapDmg = player->attack / 2 + 8 + rand() % 8;
        dealDamage(enemy, trapDmg);
        printf("\nYour trap triggers as %s attacks! %d counter-damage! (%s HP: %d/%d)\n",
               enemy->name, trapDmg, enemy->name, enemy->health, enemy->maxHealth);
        /* Trap still lets the enemy attack through */
    }

    /* Bloodmail regen */
    if (player->armorFx == AFX_BLOOD_A)
        healPlayer(player, 5);

    /* One third of Defense is subtracted from the hit, capped so that
       stacking defense can never make the hero untouchable. */
    int reduction = player->defense / 3;

    if (playerStatus[ST_BURN].turns > 0)
        reduction -= playerStatus[ST_BURN].magnitude;

    if (reduction > MAX_DEFENSE_REDUCTION)
        reduction = MAX_DEFENSE_REDUCTION;
    if (reduction < 0)
        reduction = 0;

    if (player->armorFx == AFX_AEGIS)
        reduction += 3;

    if (gBrace)
    {
        reduction += enemy->attack / 2;
        gBrace = 0;
        printf("Your brace absorbs the force of the attack.\n");
    }

    /* Iron Fortress: halve all damage */
    if (adv.ironFortressTurns > 0)
        reduction += enemy->attack / 2;

    /* Guard: halve next hit */
    if (gCombatState & CSTATE_GUARDING)
    {
        gCombatState &= ~CSTATE_GUARDING;
        reduction += enemy->attack / 2;
    }

    int enemyAttackPower = enemy->attack;
    if (enemyStatus[ST_WEAKEN].turns > 0)
        enemyAttackPower -= enemyStatus[ST_WEAKEN].magnitude;
    if (enemyAttackPower < 1) enemyAttackPower = 1;
    int damage = enemyAttackPower - reduction;
    const char *how = "attacks you";

    if (enemy->archetype == ENEMY_BRUTE && rand() % 100 < 25)
    {
        damage += enemyAttackPower / 2;
        how = "uses a crushing power attack on you";
    }
    else if (enemy->archetype == ENEMY_CASTER && rand() % 100 < 30)
    {
        applyStatus(playerStatus, ST_WEAKEN, 2, 3);
        how = "casts a weakening hex at you";
    }

    if (hasSpecial(enemy, SPECIAL_RAGE) &&
        enemy->health * 2 < enemy->maxHealth)
    {
        damage = damage * 3 / 2;
        how = "flies into a rage and strikes you";
    }

    if (hasSpecial(enemy, SPECIAL_BURN) && rand() % 100 < 20)
    {
        if (player->armorFx == AFX_DRAGON)
        {
            printf("\nYour dragon armour turns aside %s's flames!\n", enemy->name);
        }
        else
        {
            damage *= 2;
            how = "breathes fire at you";
        }
    }

    if (gShield)
    {
        gShield = 0;
        damage /= 2;
        printf("\nYour shield absorbs half of the blow!\n");
    }

    /* Frozen Heart relic: reduce fire damage taken? No, it affects player's fire */

    /* Apply Bloodstone incoming damage modifier? No, it affects HP cost for mana */

    /* Relic: reduce damage? Currently none does, but hook exists */
    damage = relicOnTakeDamage(damage);

    hitPlayer(player, enemy, damage, how);

    if (player->health > 0 && enemy->type == MT_SERPENT && rand() % 100 < 25)
        applyStatus(playerStatus, ST_POISON, 3, 3);
    else if (player->health > 0 && enemy->type == MT_IMP && rand() % 100 < 20)
        applyStatus(playerStatus, ST_BURN, 2, 3);
    else if (player->health > 0 && enemy->type == MT_FROSTWOLF && rand() % 100 < 25)
        applyStatus(playerStatus, ST_SLOW,
                    currentDungeonMod() == DMOD_FLOODED ? 3 : 2, 4);
    else if (player->health > 0 && enemy->type == MT_PIRATE && rand() % 100 < 20)
        applyStatus(playerStatus, ST_BLEED, 3, 3);

    /* Counterattack: sometimes you strike straight back */
    if (!isPlayerDefeated(player) && player->skills[SK_COUNTER] && rand() % 100 < 30)
    {
        int crit;
        int back = rollSwordDamage(player, enemy, &crit) / 2;

        if (back < 1)
            back = 1;

        dealDamage(enemy, back);
        printf("COUNTERATTACK! You strike back for %d damage. (%s HP: %d/%d)\n",
               back, enemy->name, enemy->health, enemy->maxHealth);
    }

    /* Duelist Riposte: counter for heavy damage */
    if (!isPlayerDefeated(player) && (gCombatState & CSTATE_COUNTER_READY))
    {
        gCombatState &= ~CSTATE_COUNTER_READY;
        int crit2;
        int back2 = rollSwordDamage(player, enemy, &crit2);
        dealDamage(enemy, back2);
        printf("RIPOSTE! You strike back for %d damage! (%s HP: %d/%d)\n",
               back2, enemy->name, enemy->health, enemy->maxHealth);
    }

    if (hasSpecial(enemy, SPECIAL_DOUBLE) &&
        !isPlayerDefeated(player) &&
        rand() % 100 < 25)
    {
        hitPlayer(player, enemy, enemy->attack - reduction, "strikes again");
    }

    if (hasSpecial(enemy, SPECIAL_DRAIN) && player->mana > 0)
    {
        int drain = player->mana < 5 ? player->mana : 5;

        player->mana -= drain;
        printf("%s drains %d of your mana!\n", enemy->name, drain);
    }

    /* Gambler's Coat gold bonus */
    if (player->armorFx == AFX_GAMBLE && damage > 0)
    {
        /* gold bonus applied at fight end via startCombat */
    }
}


int isEnemyDefeated(const Enemy *enemy)
{
    return enemy->health <= 0;
}


int isPlayerDefeated(const Player *player)
{
    return player->health <= 0;
}


/* ---------------------------------------------------------
   PROGRESSION
   --------------------------------------------------------- */

void gainXP(Player *player, int amount)
{
    player->xp += amount;

    printf("\nYou gained %d XP!\n", amount);

    while (player->level < LEVEL_CAP && player->xp >= player->xpToNextLevel)
    {
        player->xp -= player->xpToNextLevel;
        levelUp(player);
    }

    if (player->level >= LEVEL_CAP)
    {
        player->xp = 0;   /* at the cap, surplus XP stops piling up */
    }
}


void levelUp(Player *player)
{
    player->level++;

    player->xpToNextLevel += 35;
    player->skillPoints++;

    printf("\n================================\n");
    printf("       LEVEL UP!\n");
    printf("================================\n");

    printf("You reached Level %d (%s)!\n",
           player->level, rankName(player->level));

    printf("\nChoose an upgrade:\n");
    printf("1. Increase Attack (+10)\n");
    printf("2. Increase Defense (+10)\n");
    printf("3. Increase Max Health (+20)\n");
    printf("4. Increase Max Mana (+10)\n");

    while (1)
    {
        int choice = readCombatChoice();

        switch (choice)
        {
            case 1:
                grantOffense(player, 10);
                goto done;

            case 2:
                player->defense += 10;
                printf("Defense increased to %d!\n", player->defense);
                goto done;

            case 3:
                player->maxHealth += 20;
                player->health += 20;
                printf("Max Health increased to %d!\n", player->maxHealth);
                goto done;

            case 4:
                player->maxMana += 10;
                player->mana += 10;
                printf("Max Mana increased to %d!\n", player->maxMana);
                goto done;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }

done:
    printf("You earned a skill point! (%d unspent; spend them at the Training Area in town.)\n",
           player->skillPoints);

    /* The Path may advance a stage at certain levels. */
    pathCheckAdvance(player);
}


/* ---------------------------------------------------------
   SKILLS AND ITEMS (option 7 in a fight)
   Returns 0 = nothing happened (back to the menu),
           1 = an action was used (the enemy now acts),
           2 = you escaped the fight.
   --------------------------------------------------------- */

enum { A_POWER = 1, A_LIGHTNING, A_METEOR, A_SHIELD, A_ELIXIR, A_BOMB, A_SMOKE };

static int skillsAndItems(Player *player, Enemy *enemy, int canRun)
{
    int actions[8];
    int count = 0;

    printf("\nSkills & items:\n");

    if (player->skills[SK_POWER])
    {
        actions[count++] = A_POWER;
        printf("%d. Power Strike (8 mana)\n", count);
    }
    if (player->skills[SK_LIGHTNING])
    {
        actions[count++] = A_LIGHTNING;
        printf("%d. Lightning (12 mana, ignores armour)\n", count);
    }
    if (player->skills[SK_METEOR])
    {
        actions[count++] = A_METEOR;
        printf("%d. Meteor (30 mana)\n", count);
    }
    if (player->skills[SK_SHIELD])
    {
        actions[count++] = A_SHIELD;
        printf("%d. Shield (6 mana, halves the next hit)\n", count);
    }

    actions[count++] = A_ELIXIR;
    printf("%d. Elixir (%d left, full heal)\n", count, player->elixirs);
    actions[count++] = A_BOMB;
    printf("%d. Fire Bomb (%d left)\n", count, player->fireBombs);
    actions[count++] = A_SMOKE;
    printf("%d. Smoke Bomb (%d left, %s)\n", count, player->smokeBombs,
           canRun ? "escape" : "cannot flee this fight");
    printf("0. Back\n");

    int choice = readCombatChoice();

    if (choice < 1 || choice > count)
        return 0;

    int crit;
    int damage;

    switch (actions[choice - 1])
    {
    case A_POWER:
        if (player->mana < 8)
        {
            printf("Not enough mana!\n");
            return 0;
        }
        player->mana -= 8;
        damage = rollSwordDamage(player, enemy, &crit) * 18 / 10;
        dealDamage(enemy, damage);
        printf("\nYou unleash Power Strike for %d damage!%s\n", damage, crit ? "  CRITICAL HIT!" : "");
        printf("%s HP: %d/%d\n", enemy->name, enemy->health, enemy->maxHealth);
        weaponHitEffects(player, enemy);
        return 1;

    case A_LIGHTNING:
        if (player->mana < 12)
        {
            printf("Not enough mana!\n");
            return 0;
        }
        player->mana -= 12;
        damage = 35 + rand() % 21 + player->attack / 3;
        dealDamage(enemy, damage);
        printf("\nLightning strikes %s for %d damage!\n", enemy->name, damage);
        printf("%s HP: %d/%d\n", enemy->name, enemy->health, enemy->maxHealth);
        return 1;

    case A_METEOR:
        if (player->mana < 30)
        {
            printf("Not enough mana!\n");
            return 0;
        }
        player->mana -= 30;
        damage = 90 + rand() % 41 + player->attack / 2;
        dealDamage(enemy, damage);
        printf("\nA METEOR crashes down on %s for %d damage!\n", enemy->name, damage);
        printf("%s HP: %d/%d\n", enemy->name, enemy->health, enemy->maxHealth);
        return 1;

    case A_SHIELD:
        if (player->mana < 6)
        {
            printf("Not enough mana!\n");
            return 0;
        }
        player->mana -= 6;
        gShield = 1;
        printf("\nYou raise your shield. The next hit will be halved.\n");
        return 1;

    case A_ELIXIR:
        if (player->elixirs <= 0)
        {
            printf("You don't have any elixirs!\n");
            return 0;
        }
        player->elixirs--;
        player->health = player->maxHealth;
        printf("\nYou drink an Elixir and feel completely restored!\n");
        printf("Health: %d/%d\n", player->health, player->maxHealth);
        return 1;

    case A_BOMB:
        if (player->fireBombs <= 0)
        {
            printf("You don't have any fire bombs!\n");
            return 0;
        }
        player->fireBombs--;
        damage = 60 + rand() % 21;
        dealDamage(enemy, damage);
        printf("\nYou hurl a Fire Bomb! It deals %d damage!\n", damage);
        printf("%s HP: %d/%d\n", enemy->name, enemy->health, enemy->maxHealth);
        return 1;

    case A_SMOKE:
        if (player->smokeBombs <= 0)
        {
            printf("You don't have any smoke bombs!\n");
            return 0;
        }
        if (!canRun)
        {
            printf("There is no escaping this fight, not even with smoke!\n");
            return 0;
        }
        player->smokeBombs--;
        printf("\nYou throw a Smoke Bomb and vanish from the battle!\n");
        return 2;
    }

    return 0;
}


/* ---------------------------------------------------------
   A WHOLE FIGHT
   --------------------------------------------------------- */

int startCombat(Player *player, Enemy *enemy, int canRun)
{
    if (adv.difficulty == 0)
    {
        enemy->maxHealth = enemy->maxHealth * 85 / 100;
        enemy->health = enemy->maxHealth;
        enemy->attack = enemy->attack * 85 / 100;
    }
    else if (adv.difficulty == 2)
    {
        enemy->maxHealth = enemy->maxHealth * 125 / 100;
        enemy->health = enemy->maxHealth;
        enemy->attack = enemy->attack * 115 / 100;
    }
    gShield = 0;
    gFrozen = 0;
    gIronWillUsed = 0;
    gStunTurns = 0;
    gSnareTurns = 0;
    gCombatState = 0;
    gDmgThisTurn = 0;
    gBrace = 0;
    gGuard = 0;
    gStamina = 3;
    gHasteExtra = 0;
    gSkillCooldown = 0;
    memset(enemyStatus, 0, sizeof enemyStatus);
    memset(playerStatus, 0, sizeof playerStatus);
    if (currentDungeonMod() == DMOD_HAUNTED)
        applyStatus(playerStatus, ST_WEAKEN, 2, 3);

    /* Reset per-fight class combat state in Adventure */
    adv.ironFortressTurns = 0;
    adv.warcryTurns = 0;
    adv.burnTurns = 0; adv.burnDamage = 0;
    adv.bleedTurns = 0; adv.bleedDamage = 0;
    adv.poisonTurns = 0; adv.poisonDamage = 0;
    adv.stormTurns = 0; adv.stormDamage = 0;
    adv.markTurns = 0;
    adv.huntersFocusTurns = 0;
    adv.snareTurns = 0;
    adv.grandTrapStun = 0;
    adv.shadowStepTurns = 0;
    adv.bloodPactTurns = 0;
    adv.forbiddenPowerTurns = 0;
    adv.pactSummonTurns = 0;
    adv.pactSummonDamage = 0;
    adv.highRollerTurns = 0;
    adv.stackedDeckTurns = 0;
    adv.precisionActive = 0;
    adv.commandActive = 0;
    adv.warlordsMightActive = 0;
    adv.sacrificeActive = 0;
    adv.disorientActive = 0;
    adv.wildCardBuff = 0;
    adv.cardSharpBuff = 0;
    adv.doubleDownActive = 0;
    adv.instantKill = 0;
    adv.cryoFreeze = 0;
    adv.cryoFreeze2 = 0;

    if (adv.vowReady && !adv.vowUsed)
    {
        adv.pactSummonTurns = 3;
        adv.pactSummonDamage = 8 + player->level;
        adv.vowReady = 0;
        adv.vowUsed = 1;
        printf("Galahad joins your side for three strikes!\n");
    }
    if (adv.loanReady)
    {
        if (adv.loanMana > 0)
        {
            restoreMana(player, adv.loanMana);
            printf("Morgana's Loan restores %d MP.\n", adv.loanMana);
        }
        if (adv.loanShield > 0)
        {
            gShield = 1;
            printf("Morgana's Loan raises a one-hit magic shield.\n");
        }
    }

    /* Set class evasion */
    player->evasion = classEvasionBonus(adv.playerClass);

#ifdef RPG_TUI
    asciiCombatHeader(enemy->name, enemy->level);
#endif
    printf("\n");
    uiRule();
    uiTitle("COMBAT ENCOUNTER");
    uiRule();

    printf("You are fighting: ");
    printEnemyTag(enemy);
    printf("\n");

    if (specialText(enemy->special) != NULL)
        printf("%s\n", specialText(enemy->special));
    if (specialText(enemy->special2) != NULL)
        printf("%s\n", specialText(enemy->special2));

    int initiative = adv.gateInitiativeDelta + classInitiativeBonus(adv.playerClass);
    if (playerStatus[ST_SLOW].turns > 0) initiative -= playerStatus[ST_SLOW].magnitude;
    if (playerStatus[ST_HASTE].turns > 0) initiative += playerStatus[ST_HASTE].magnitude;
    if (initiative < 0)
    {
        printf("Your slow approach costs the first move.\n");
        enemyAttack(player, enemy);
    }
    else if (initiative > 0 && adv.gateChoice == 2)
    {
        int first = rawWeaponDamage(player, enemy) / 3 + initiative / 3;
        if (first < 1) first = 1;
        dealDamage(enemy, first);
        printf("You seize the initiative with a first strike for %d damage!\n", first);
        if (adv.playerClass == CLASS_ROGUE)
            applyStatus(enemyStatus, ST_STUN, 1, 1);
    }

    if (adv.darkReady && !adv.darkUsed)
    {
        int opening = rawWeaponDamage(player, enemy) * 2;
        dealDamage(enemy, opening);
        adv.darkReady = 0;
        adv.darkUsed = 1;
        printf("Batman strikes from the dark for a guaranteed critical hit (%d damage)!\n", opening);
    }

    while (!isEnemyDefeated(enemy) && !isPlayerDefeated(player))
    {
        printf("\n");
        uiRule();
        uiTitle("YOUR TURN");
        uiRule();

        tickPlayerStatus(player);
        tickEnemyStatus(enemy);
        if (isEnemyDefeated(enemy) || isPlayerDefeated(player)) break;

        /* --- Apply DOTs at turn start --- */
        gDmgThisTurn = 0;

        if (adv.burnTurns > 0)
        {
            int dmg = adv.burnDamage;
            dealDamage(enemy, dmg);
            adv.burnTurns--;
            gDmgThisTurn += dmg;
            printf("%s burns for %d damage! (%d left)\n", enemy->name, dmg, adv.burnTurns);
        }
        if (adv.bleedTurns > 0)
        {
            int dmg = adv.bleedDamage;
            dealDamage(enemy, dmg);
            adv.bleedTurns--;
            gDmgThisTurn += dmg;
            printf("%s bleeds for %d damage! (%d left)\n", enemy->name, dmg, adv.bleedTurns);
        }
        if (adv.poisonTurns > 0)
        {
            int dmg = adv.poisonDamage;
            dealDamage(enemy, dmg);
            adv.poisonTurns--;
            gDmgThisTurn += dmg;
            printf("%s takes %d poison damage! (%d left)\n", enemy->name, dmg, adv.poisonTurns);
        }
        if (adv.stormTurns > 0)
        {
            int dmg = adv.stormDamage + rand() % 6;
            dealDamage(enemy, dmg);
            adv.stormTurns--;
            gDmgThisTurn += dmg;
            printf("Lightning strikes %s for %d damage! (%d bolts left)\n", enemy->name, dmg, adv.stormTurns);
        }
        if (adv.pactSummonTurns > 0)
        {
            int dmg = adv.pactSummonDamage;
            dealDamage(enemy, dmg);
            adv.pactSummonTurns--;
            gDmgThisTurn += dmg;
            printf("Your dark ally strikes %s for %d damage! (%d turns left)\n", enemy->name, dmg, adv.pactSummonTurns);
        }

        /* Forbidden Power: take 10% max HP damage per turn */
        if (adv.forbiddenPowerTurns > 0)
        {
            int recoil = player->maxHealth / 10;
            if (recoil < 5) recoil = 5;
            damagePlayer(player, recoil);
            adv.forbiddenPowerTurns--;
            printf("Forbidden Power sears you for %d damage! HP: %d/%d (%d turns left)\n",
                   recoil, player->health, player->maxHealth, adv.forbiddenPowerTurns);
        }

        /* Decrement buff timers */
        if (adv.warcryTurns > 0) adv.warcryTurns--;
        if (adv.ironFortressTurns > 0) adv.ironFortressTurns--;
        if (adv.markTurns > 0) adv.markTurns--;
        if (adv.huntersFocusTurns > 0) adv.huntersFocusTurns--;
        if (adv.bloodPactTurns > 0) adv.bloodPactTurns--;
        if (adv.highRollerTurns > 0) adv.highRollerTurns--;
        if (adv.stackedDeckTurns > 0) adv.stackedDeckTurns--;

        /* Grand trap stun decrement */
        if (adv.grandTrapStun > 0)
        {
            gStunTurns = adv.grandTrapStun;
            adv.grandTrapStun = 0;
        }

        /* Cryo-freeze effects (from freeze spell / absolute zero) */
        if (adv.cryoFreeze2 > 0)
        {
            adv.cryoFreeze2--;
            gFrozen = 1;  /* enemy loses this turn too */
        }

        /* Check if DOTs killed the enemy */
        if (isEnemyDefeated(enemy))
            break;

        /* Check if Forbidden Power killed the player */
        if (isPlayerDefeated(player))
            break;

        /* --- Telegraph hint (Eye of Oracle) --- */
        if (adv.relic == RELIC_EYE_OF_ORACLE)
        {
            Telegraph t = rollTelegraph(enemy);
            const char *hint = telegraphText(t);
            if (hint)
                printf("\n[Eye of the Oracle] %s\n", hint);
        }

        char targetLine[160];
        snprintf(targetLine, sizeof targetLine, "Enemy: %s | HP %d/%d",
                 enemy->name, enemy->health, enemy->maxHealth);
        uiPanelLine(targetLine);
        printf("%s HP: %d/%d   Mana: %d/%d\n",
               player->name, player->health, player->maxHealth,
               player->mana, player->maxMana);

        /* Show active buff indicators */
        if (adv.warcryTurns > 0 || adv.ironFortressTurns > 0 ||
            adv.markTurns > 0 || adv.huntersFocusTurns > 0 ||
            adv.bloodPactTurns > 0 || adv.forbiddenPowerTurns > 0 ||
            adv.highRollerTurns > 0 || adv.shadowStepTurns > 0)
        {
            printf("Active: ");
            int first = 1;
            if (adv.warcryTurns > 0) { printf("%sWar Cry(%d)", first ? "" : ", ", adv.warcryTurns); first = 0; }
            if (adv.ironFortressTurns > 0) { printf("%sIron Fortress(%d)", first ? "" : ", ", adv.ironFortressTurns); first = 0; }
            if (adv.markTurns > 0) { printf("%sMarked(%d)", first ? "" : ", ", adv.markTurns); first = 0; }
            if (adv.huntersFocusTurns > 0) { printf("%sFocus(%d)", first ? "" : ", ", adv.huntersFocusTurns); first = 0; }
            if (adv.bloodPactTurns > 0) { printf("%sBlood Pact(%d)", first ? "" : ", ", adv.bloodPactTurns); first = 0; }
            if (adv.forbiddenPowerTurns > 0) { printf("%sForbidden Power(%d)", first ? "" : ", ", adv.forbiddenPowerTurns); first = 0; }
            if (adv.highRollerTurns > 0) { printf("%sHigh Roller(%d)", first ? "" : ", ", adv.highRollerTurns); first = 0; }
            if (adv.shadowStepTurns > 0) { printf("%sShadow Step(%d)", first ? "" : ", ", adv.shadowStepTurns); first = 0; }
            printf("\n");
        }

        printf("\nChoose your action:\n");
        printf("1. Attack  (%s)\n", weaponTypeName(weaponTypeOf(adv.weapon)));
        printf("9. Power Attack (2 focus, lower accuracy)\n");
        printf("10. Brace (reduce this hit; build guard %d/3)\n", gGuard);
        if (adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK)
        {
            printf("2. Cast Fireball (10 MP)\n");
            printf("3. Cast Heal (10 MP)\n");
        }
        else
        {
            printf("2. Cast (casters only)\n");
            printf("3. Cast (casters only)\n");
        }
        printf("4. Health Potion (%d left)\n", player->potions);
        printf("5. Mana Potion (%d left)\n", player->manaPotions);
        printf("6. %s\n", canRun ? "Run" : "Run (no escape from this fight!)");
        printf("7. Skills & items\n");

        /* Show class ability option if player has any class abilities */
        int hasClassAb = 0;
        for (int ab = 0; ab < AB_COUNT; ab++)
        {
            if (adv.classAbilities[ab]) { hasClassAb = 1; break; }
        }
        if (hasClassAb)
            printf("8. Class Abilities\n");

        int choice = readCombatChoice();

        switch (choice)
        {
            case 1:
                if (!weaponAttackMenu(player, enemy))
                    continue;   /* backed out of the moveset */
                if (adv.playerClass == CLASS_RANGER && rand() % 100 < 35)
                    applyStatus(enemyStatus, ST_POISON, 3, 4);
                else if (adv.playerClass == CLASS_ROGUE && rand() % 100 < 25)
                {
                    applyStatus(enemyStatus, ST_BLEED, 3, 2);
                    applyStatus(enemyStatus, ST_VULNERABLE, 2, 20);
                }
                else if (adv.playerClass == CLASS_GAMBLER && rand() % 100 < 20)
                    applyStatus(playerStatus, ST_HASTE, 2, 4);
                if (gGuard >= 2 && !isEnemyDefeated(enemy))
                {
                    int counter = player->attack / 2;
                    gGuard = 0;
                    dealDamage(enemy, counter);
                    printf("You spend your guard on a counter for %d damage!\n", counter);
                }
                break;

            case 2:
            {
                if (adv.playerClass != CLASS_ARCANIST && adv.playerClass != CLASS_WARLOCK)
                {
                    printf("Only an Arcanist or Warlock can cast this spell.\n");
                    continue;
                }
                if (player->mana < 10)
                {
                    printf("Not enough mana!\n");
                    continue;
                }

                int damage = 20 + rand() % 31 + player->attack / 4 + adv.gateSpellDelta;
                if (damage < 1) damage = 1;

                if (player->skills[SK_FIRE])
                    damage += 12;

                player->mana -= 10;
                dealDamage(enemy, damage);
                if (adv.playerClass == CLASS_ARCANIST)
                    applyStatus(enemyStatus, ST_BURN, 3, 4);
                else
                {
                    applyStatus(enemyStatus, ST_BLEED, 3, 2);
                    applyStatus(enemyStatus, ST_WEAKEN, 2, 4);
                    healPlayer(player, damage / 5);
                }

                printf("\nYou cast Fireball!\n");
                printf("You deal %d damage!\n", damage);
                printf("%s HP: %d/%d\n", enemy->name,
                       enemy->health, enemy->maxHealth);
                break;
            }

            case 3:
            {
                if (adv.playerClass != CLASS_ARCANIST && adv.playerClass != CLASS_WARLOCK)
                {
                    printf("Only an Arcanist or Warlock can cast this spell.\n");
                    continue;
                }
                if (player->mana < 10)
                {
                    printf("Not enough mana!\n");
                    continue;
                }

                player->mana -= 10;
                healPlayer(player, player->healPower);
                applyStatus(playerStatus, ST_REGEN, 2, 5);

                printf("\nYou cast Heal!\n");
                printf("Health: %d/%d\n", player->health, player->maxHealth);
                break;
            }

            case 4:
            {
                if (player->potions <= 0)
                {
                    printf("You don't have any health potions!\n");
                    continue;
                }

                player->potions--;
                healPlayer(player, 50);

                printf("\nYou used a Health Potion!\n");
                printf("Health: %d/%d\n", player->health, player->maxHealth);
                break;
            }

            case 5:
            {
                if (player->manaPotions <= 0)
                {
                    printf("You don't have any mana potions!\n");
                    continue;
                }

                player->manaPotions--;
                restoreMana(player, 20);

                printf("\nYou used a Mana Potion!\n");
                printf("Mana: %d/%d\n", player->mana, player->maxMana);
                break;
            }

            case 6:
                if (!canRun)
                {
                    printf("\nThere is no escape from this fight!\n");
                    continue;
                }

                if (rand() % 100 < RUN_SUCCESS_PERCENT)
                {
                    printf("\nYou escaped from the battle!\n");
                    return 0;
                }

                printf("\nYou try to run, but %s cuts you off!\n", enemy->name);
                break;              /* the enemy now gets a free attack */

            case 7:
            {
                int result = skillsAndItems(player, enemy, canRun);

                if (result == 0)
                    continue;       /* nothing used: back to the menu */

                if (result == 2)
                    return 0;       /* escaped with a smoke bomb */

                break;
            }

            case 8:
            {
                if (gSkillCooldown > 0)
                {
                    printf("Your class skill is recovering for %d more turn(s).\n", gSkillCooldown);
                    continue;
                }
                /* Class ability menu */
                int enemyHP = (enemy->maxHealth > 0) ?
                    enemy->health * 100 / enemy->maxHealth : 0;
                Telegraph tele = TELE_NONE;
                if (adv.relic == RELIC_EYE_OF_ORACLE)
                    tele = rollTelegraph(enemy);

                int result = classAbilityMenu(&gCombatState, tele,
                    1, /* playerActingFirst: simplified */
                    gFrozen, 0,
                    enemyHP);

                if (result == 0)
                    continue;   /* no ability used */
                gSkillCooldown = 2;

                /* Check for instant kill from class ability */
                if (adv.instantKill)
                {
                    adv.instantKill = 0;
                    enemy->health = 0;
                    printf("The enemy is instantly defeated!\n");
                    break;
                }

                /* Shield Bash stun */
                /* handled via adv.grandTrapStun or gStunTurns set by ability */

                break;
            }

            case 9:
                if (gStamina < 2)
                {
                    printf("You need 2 focus for a Power Attack.\n");
                    continue;
                }
                gStamina -= 2;
                if (rand() % 100 >= 75)
                {
                    printf("Your heavy swing misses, leaving you open!\n");
                    break;
                }
                {
                    int damage = rawWeaponDamage(player, enemy) * 3 / 2;
                    int crit = 0;
                    damage = applyCombatBuffs(player, enemy, damage, &crit);
                    dealDamage(enemy, damage);
                    printf("Power Attack deals %d damage%s. Focus: %d/3\n",
                           damage, crit ? " (critical)" : "", gStamina);
                }
                break;

            case 10:
                gBrace = 1;
                if (gGuard < 3) gGuard++;
                applyStatus(playerStatus, ST_SHIELD, 1, 8);
                printf("You brace. Guard rises to %d/3.\n", gGuard);
                break;

            default:
                printf("Invalid choice.\n");
                continue;
        }

        /* The enemy gets a turn unless it died. */
        if (!isEnemyDefeated(enemy))
        {
            if (playerStatus[ST_HASTE].turns > 0 && rand() % 100 < 25)
            {
                printf("Haste grants you another action before the enemy can respond!\n");
                continue;
            }
            if (hasSpecial(enemy, SPECIAL_REGEN))
            {
                int heal = 3 + enemy->level;

                enemy->health += heal;
                if (enemy->health > enemy->maxHealth)
                    enemy->health = enemy->maxHealth;

                printf("%s regenerates %d HP.\n", enemy->name, heal);
            }

            enemyAttack(player, enemy);
            if (gSkillCooldown > 0) gSkillCooldown--;
            if (gStamina < 3) gStamina++;
        }
    }

    if (isEnemyDefeated(enemy))
    {
#ifdef RPG_TUI
        asciiVictory();
#endif
        printf("\n================================\n");
        printf("          VICTORY!\n");
        printf("================================\n");

        printf("You defeated %s!\n", enemy->name);

        int goldReward = enemy->goldReward;

        /* Gambler's Coat: +20% gold */
        if (player->armorFx == AFX_GAMBLE)
            goldReward = goldReward * 12 / 10;

        /* Gambler class: small gold bonus */
        if (adv.playerClass == CLASS_GAMBLER)
            goldReward += goldReward / 10;

        printf("Gold gained: %d\n", goldReward);
        player->gold += goldReward;

        gainXP(player, enemy->xpReward);
        onEnemyDefeated(enemy);

        return 1;
    }

#ifdef RPG_TUI
    asciiDefeat();
#endif
    printf("\n================================\n");
    printf("          DEFEAT\n");
    printf("================================\n");

    printf("You were defeated by %s.\n", enemy->name);

    return -1;
}
