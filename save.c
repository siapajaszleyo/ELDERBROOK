#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "save.h"
#include "adventure.h"
#include "forge.h"

#define SAVE_HEADER_V1 "RPGSAVE 1"
#define SAVE_HEADER_V2 "RPGSAVE 2"
#define SAVE_HEADER_V3 "RPGSAVE 3"
#define SAVE_HEADER_V4 "RPGSAVE 4"
#define SAVE_HEADER_V5 "RPGSAVE 5"
#define SAVE_HEADER_V6 "RPGSAVE 6"
#define SAVE_HEADER_V7 "RPGSAVE 7"

/* ---------------------------------------------------------
   THE FIELD TABLE
   Every saved number is listed once here: its name in the file, where
   it lives in memory, how many numbers it has, and the range that is
   considered sane. The same table drives saving, loading AND checking,
   so adding a new saved value later is a single line.
   --------------------------------------------------------- */

typedef struct
{
    const char *key;
    int        *where;
    int         count;
    int         min;
    int         max;
} Field;

static int buildFields(Field *f, Player *p, Progress *g, Adventure *a)
{
    int n = 0;

#define ADD(k, w, c, lo, hi) \
    do { f[n].key = k; f[n].where = (int *)(w); f[n].count = c; \
         f[n].min = lo; f[n].max = hi; n++; } while (0)

    /* the hero */
    ADD("health",          &p->health,         1, 1, 5000);
    ADD("maxHealth",       &p->maxHealth,      1, 1, 5000);
    ADD("mana",            &p->mana,           1, 0, 5000);
    ADD("maxMana",         &p->maxMana,        1, 0, 5000);
    ADD("attack",          &p->attack,         1, 0, 9999);
    ADD("defense",         &p->defense,        1, 0, 9999);
    ADD("healPower",       &p->healPower,      1, 0, 9999);
    ADD("potions",         &p->potions,        1, 0, 999);
    ADD("manaPotions",     &p->manaPotions,    1, 0, 999);
    ADD("gold",            &p->gold,           1, 0, 9999999);
    ADD("level",           &p->level,          1, 1, 99);
    ADD("xp",              &p->xp,             1, 0, 999999);
    ADD("xpToNextLevel",   &p->xpToNextLevel,  1, 1, 999999);
    ADD("knightChosen",    &p->knightChosen,   1, 0, 3);
    ADD("swordUpgrades",   &p->swordUpgrades,  1, 0, 99);
    ADD("keys",            p->keys,            NUM_KEYS, 0, 1);
    ADD("critChance",      &p->critChance,     1, 0, 100);
    ADD("weaponFx",        &p->weaponFx,       1, 0, 20);
    ADD("armorFx",         &p->armorFx,        1, 0, 20);
    ADD("skillPoints",     &p->skillPoints,    1, 0, 99);
    ADD("skills",          p->skills,          SKILL_COUNT, 0, 1);
    ADD("elixirs",         &p->elixirs,        1, 0, 99);
    ADD("fireBombs",       &p->fireBombs,      1, 0, 99);
    ADD("smokeBombs",      &p->smokeBombs,     1, 0, 99);

    /* where the hero is in the adventure */
    ADD("dungeonsCleared", &g->dungeonsCleared, 1, 0, DUNGEON_COUNT);
    ADD("currentDungeon",  &g->currentDungeon,  1, 0, DUNGEON_COUNT);
    ADD("planCount",       &g->planCount,       1, 0, MAX_PLAN);
    ADD("planIndex",       &g->planIndex,       1, 0, MAX_PLAN);
    ADD("plan",            g->plan,             MAX_PLAN, 0, EV_COUNT - 1);
    ADD("revisit",         &g->revisit,         1, 0, 1);

    /* the Expanded Edition */
    ADD("weapon",          &a->weapon,          1, 0, ITEM_COUNT - 1);
    ADD("armor",           &a->armor,           1, 0, ITEM_COUNT - 1);
    ADD("owned",           a->owned,            ITEM_COUNT, 0, 1);
    ADD("materials",       a->materials,        MAT_COUNT, 0, 999);
    ADD("questState",      a->questState,       QUEST_COUNT, 0, 2);
    ADD("questProgress",   a->questProgress,    QUEST_COUNT, 0, 99999);
    ADD("npcMet",          a->npcMet,           NPC_COUNT, 0, 1);
    ADD("affinity",        a->affinity,         NPC_COUNT, 0, 30);
    ADD("achieved",        a->achieved,         ACH_COUNT, 0, 1);
    ADD("secrets",         a->secrets,          SECRET_COUNT, 0, 1);
    ADD("optBoss",         a->optBoss,          OPTBOSS_COUNT, 0, 1);
    ADD("visited",         a->visited,          DUNGEON_COUNT, 0, 1);
    ADD("training",        a->training,         3, 0, 9);
    ADD("flags",           a->flags,            FLAG_COUNT, 0, 9);
    ADD("stats",           a->stats,            STAT_COUNT, 0, 9999999);
    ADD("rodLevel",        &a->rodLevel,        1, 0, 9);

    /* ---- Character 2.0 ---- */
    ADD("playerClass",     &a->playerClass,     1, 0, 6);       /* CLASS_NONE..CLASS_GAMBLER */
    ADD("playerDiscipline",&a->playerDiscipline, 1, 0, 19);     /* DISC_NONE..DISC_DEALER     */
    ADD("classAbilities",  a->classAbilities,   AB_COUNT_MAX, 0, 1);
    ADD("relic",           &a->relic,           1, 0, 7);       /* RELIC_NONE..RELIC_DRAGON_TOOTH */

    /* ---- Phase 4: forge enchants ---- */
    ADD("weaponEnchant",   &a->weaponEnchant,   1, 0, ENC_COUNT - 1);
    ADD("armorEnchant",    &a->armorEnchant,    1, 0, ENC_COUNT - 1);

    /* ---- Phase 3: guilds & factions ---- */
    ADD("guildRank",       a->guildRank,        GUILD_COUNT, 0, 6);
    ADD("guildCreds",      a->guildCreds,       GUILD_COUNT, 0, 999999);
    ADD("guildStanding",   a->guildStanding,    GUILD_COUNT, -100, 100);
    ADD("guildQuestStage", a->guildQuestStage,  GUILD_COUNT, 0, 3);

    /* Combat-duration buffs/debuffs */
    ADD("ironFortressTurns",&a->ironFortressTurns, 1, 0, 99);
    ADD("warcryTurns",     &a->warcryTurns,     1, 0, 99);
    ADD("burnTurns",       &a->burnTurns,       1, 0, 99);
    ADD("burnDamage",      &a->burnDamage,      1, 0, 9999);
    ADD("bleedTurns",      &a->bleedTurns,      1, 0, 99);
    ADD("bleedDamage",     &a->bleedDamage,     1, 0, 9999);
    ADD("poisonTurns",     &a->poisonTurns,     1, 0, 99);
    ADD("poisonDamage",    &a->poisonDamage,    1, 0, 9999);
    ADD("stormTurns",      &a->stormTurns,      1, 0, 99);
    ADD("stormDamage",     &a->stormDamage,     1, 0, 9999);
    ADD("markTurns",       &a->markTurns,       1, 0, 99);
    ADD("huntersFocusTurns",&a->huntersFocusTurns,1, 0, 99);
    ADD("snareTurns",      &a->snareTurns,      1, 0, 99);
    ADD("grandTrapStun",   &a->grandTrapStun,   1, 0, 99);
    ADD("shadowStepTurns", &a->shadowStepTurns, 1, 0, 99);
    ADD("bloodPactTurns",  &a->bloodPactTurns,  1, 0, 99);
    ADD("forbiddenPowerTurns",&a->forbiddenPowerTurns,1, 0, 99);
    ADD("pactSummonTurns", &a->pactSummonTurns, 1, 0, 99);
    ADD("pactSummonDamage",&a->pactSummonDamage,1, 0, 9999);
    ADD("highRollerTurns", &a->highRollerTurns, 1, 0, 99);
    ADD("stackedDeckTurns",&a->stackedDeckTurns,1, 0, 99);

    /* One-shot combat flags */
    ADD("precisionActive", &a->precisionActive, 1, 0, 1);
    ADD("commandActive",   &a->commandActive,   1, 0, 1);
    ADD("warlordsMightActive",&a->warlordsMightActive,1,0,1);
    ADD("sacrificeActive", &a->sacrificeActive, 1, 0, 1);
    ADD("disorientActive", &a->disorientActive, 1, 0, 1);
    ADD("wildCardBuff",    &a->wildCardBuff,    1, 0, 99);
    ADD("cardSharpBuff",   &a->cardSharpBuff,   1, 0, 99);
    ADD("doubleDownActive",&a->doubleDownActive,1, 0, 1);
    ADD("instantKill",     &a->instantKill,     1, 0, 1);
    ADD("cryoFreeze",      &a->cryoFreeze,      1, 0, 1);
    ADD("cryoFreeze2",     &a->cryoFreeze2,     1, 0, 1);

    /* Player class/relic fields */
    ADD("p_class",         &p->playerClass,     1, 0, 6);
    ADD("p_discipline",    &p->playerDiscipline, 1, 0, 19);
    ADD("p_relic",         &p->relic,           1, 0, 7);
    ADD("p_evasion",       &p->evasion,         1, 0, 100);
    ADD("p_pathStage",     &p->pathStage,       1, 0, 2);

    ADD("gateAttackDelta", &a->gateAttackDelta, 1, -50, 50);
    ADD("gateDefenseDelta",&a->gateDefenseDelta,1, -50, 50);
    ADD("gateHealthDelta", &a->gateHealthDelta, 1, -200, 200);
    ADD("gateManaDelta",   &a->gateManaDelta,   1, -100, 100);
    ADD("gateCritDelta",   &a->gateCritDelta,   1, -50, 50);
    ADD("gateInitiativeDelta",&a->gateInitiativeDelta,1,-50,50);
    ADD("gateSpellDelta",   &a->gateSpellDelta, 1, -10, 10);
    ADD("gateChoice",      &a->gateChoice,      1, 0, 3);
    ADD("difficulty",      &a->difficulty,      1, 0, 2);
    ADD("endlessDepth",    &a->endlessDepth,    1, 0, 999999);
    ADD("vowReady",        &a->vowReady,        1, 0, 1);
    ADD("loanReady",       &a->loanReady,       1, 0, 1);
    ADD("darkReady",       &a->darkReady,       1, 0, 1);
    ADD("vowUsed",         &a->vowUsed,         1, 0, 1);
    ADD("loanUsed",        &a->loanUsed,        1, 0, 1);
    ADD("darkUsed",        &a->darkUsed,        1, 0, 1);
    ADD("loanMana",        &a->loanMana,        1, 0, 100);
    ADD("loanShield",      &a->loanShield,      1, 0, 100);

#undef ADD
    return n;
}

#define MAX_FIELDS 160


/* ---------------------------------------------------------
   WRITING
   To avoid ever ending up with half a save file, we write to a
   temporary file first, and only then swap it into place.
   --------------------------------------------------------- */

static int copyFile(const char *from, const char *to)
{
    FILE *in = fopen(from, "rb");
    if (in == NULL)
        return 0;

    FILE *out = fopen(to, "wb");
    if (out == NULL)
    {
        fclose(in);
        return 0;
    }

    char buffer[1024];
    size_t count;

    while ((count = fread(buffer, 1, sizeof buffer, in)) > 0)
        fwrite(buffer, 1, count, out);

    fclose(in);
    return fclose(out) == 0;
}

int saveGame(const Player *p, const Progress *g)
{
    Field fields[MAX_FIELDS];
    int n = buildFields(fields, (Player *)p, (Progress *)g, &adv);

    FILE *f = fopen(SAVE_TEMP, "w");
    if (f == NULL)
        return 0;

    fprintf(f, "%s\n", SAVE_HEADER_V7);
    fprintf(f, "name %s\n", p->name);

    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%s", fields[i].key);

        for (int k = 0; k < fields[i].count; k++)
            fprintf(f, " %d", fields[i].where[k]);

        fprintf(f, "\n");
    }

    fprintf(f, "END\n");

    if (fclose(f) != 0)
    {
        remove(SAVE_TEMP);
        return 0;
    }

    /* The old main save becomes the backup (fine if there wasn't one) */
    copyFile(SAVE_FILE, SAVE_BACKUP);

    /* Windows will not rename over an existing file, so remove first */
    remove(SAVE_FILE);

    return rename(SAVE_TEMP, SAVE_FILE) == 0;
}


/* ---------------------------------------------------------
   READING
   Never trust a file: check every value is sensible.
   --------------------------------------------------------- */

static int validate(const Field *fields, int n, const Player *p, const Progress *g)
{
    if (p->name[0] == '\0')
        return 0;

    for (int i = 0; i < n; i++)
        for (int k = 0; k < fields[i].count; k++)
            if (fields[i].where[k] < fields[i].min ||
                fields[i].where[k] > fields[i].max)
                return 0;

    if (p->health > p->maxHealth)       return 0;
    if (p->mana > p->maxMana)           return 0;
    if (g->planIndex > g->planCount)    return 0;
    if (g->currentDungeon > 0 && g->planCount == 0)  return 0;

    if (adv.weapon != 0 && !adv.owned[adv.weapon])   return 0;
    if (adv.armor  != 0 && !adv.owned[adv.armor])    return 0;

    return 1;
}

static int parseSave(const char *path, Player *p, Progress *g)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return 0;

    char line[512];

    if (fgets(line, sizeof line, f) == NULL ||
        (strncmp(line, SAVE_HEADER_V1, strlen(SAVE_HEADER_V1)) != 0 &&
         strncmp(line, SAVE_HEADER_V2, strlen(SAVE_HEADER_V2)) != 0 &&
         strncmp(line, SAVE_HEADER_V3, strlen(SAVE_HEADER_V3)) != 0 &&
         strncmp(line, SAVE_HEADER_V4, strlen(SAVE_HEADER_V4)) != 0 &&
         strncmp(line, SAVE_HEADER_V5, strlen(SAVE_HEADER_V5)) != 0 &&
         strncmp(line, SAVE_HEADER_V6, strlen(SAVE_HEADER_V6)) != 0 &&
         strncmp(line, SAVE_HEADER_V7, strlen(SAVE_HEADER_V7)) != 0))
    {
        fclose(f);
        return 0;
    }

    /* Start from the defaults; a version 1 file simply never mentions
       the newer fields, so they keep these starting values. */
    initializePlayer(p, "Hero");
    initializeAdventure();
    memset(g, 0, sizeof *g);

    Field fields[MAX_FIELDS];
    int n = buildFields(fields, p, g, &adv);
    int sawEnd = 0;

    while (fgets(line, sizeof line, f) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "END") == 0)
        {
            sawEnd = 1;
            break;
        }

        char key[32];
        int used = 0;

        if (sscanf(line, "%31s %n", key, &used) < 1)
            continue;

        const char *rest = line + used;

        if (strcmp(key, "name") == 0)
        {
            strncpy(p->name, rest, sizeof p->name - 1);
            p->name[sizeof p->name - 1] = '\0';
            continue;
        }

        for (int i = 0; i < n; i++)
        {
            if (strcmp(fields[i].key, key) != 0)
                continue;

            const char *cursor = rest;

            for (int k = 0; k < fields[i].count; k++)
            {
                int value, consumed;

                if (sscanf(cursor, "%d%n", &value, &consumed) != 1)
                    break;

                fields[i].where[k] = value;
                cursor += consumed;
            }
            break;
        }
    }

    fclose(f);

    return sawEnd && validate(fields, n, p, g);
}

int loadGame(Player *p, Progress *g)
{
    if (parseSave(SAVE_FILE, p, g))
        return 1;

    if (parseSave(SAVE_BACKUP, p, g))
        return 2;

    return 0;
}

int saveExists(void)
{
    FILE *f = fopen(SAVE_FILE, "r");

    if (f == NULL)
        f = fopen(SAVE_BACKUP, "r");

    if (f == NULL)
        return 0;

    fclose(f);
    return 1;
}

void deleteSave(void)
{
    remove(SAVE_FILE);
    remove(SAVE_BACKUP);
    remove(SAVE_TEMP);
}
