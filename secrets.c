#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "secrets.h"
#include "adventure.h"
#include "world.h"
#include "items.h"
#include "npc.h"
#include "quest.h"
#include "achievements.h"
#include "combat.h"
#include "memory.h"
#include "reflex.h"
#include "lockpick.h"
#include "input.h"
#include "ui.h"
#include "class.h"

/* =========================================================
   SECRET ROOMS
   One per early location. Each hides a different puzzle: a riddle,
   a lever combination, a lock, a memory trial or a reflex test.
   They are optional, and a failed attempt can simply be retried the
   next time the room turns up.
   ========================================================= */

enum { PZ_RIDDLE, PZ_LEVERS, PZ_LOCK, PZ_MEMORY, PZ_REFLEX };

typedef struct
{
    const char *name;
    const char *intro;
    int type;
    const char *question;       /* riddle text or lever clue */
    const char *answer;         /* riddle keyword or lever order */
    int level;                  /* difficulty for lock/memory/reflex */
} Secret;

static const Secret secrets[SECRET_COUNT] =
{
 /* 0 */ { "the Hidden Statue",
    "Behind a curtain of ivy, a mossy statue's mouth is open as if about to speak.",
    PZ_RIDDLE, "\"I speak without a mouth and hear without ears. I have no body, but I come alive with the wind. What am I?\"", "echo", 0 },
 /* 1 */ { "the Strange Wall",
    "One wall of the old cellar has three iron levers set into it: RED, BLUE and GREEN.",
    PZ_LEVERS, "Scratched into the stone: \"Blue before green. Red comes last.\"", "bgr", 0 },
 /* 2 */ { "the Sealed Vault",
    "A round stone door, covered in dials, stands at the end of a dripping corridor.",
    PZ_LOCK, "", "", 3 },
 /* 3 */ { "Galahad's Cell",
    "A hidden door in the armoury wall swings inward. A figure in dented armour slumps in a cell, behind a lock of coloured lights.",
    PZ_MEMORY, "", "", 4 },
 /* 4 */ { "the Hidden Hoard",
    "A narrow crack opens into a cavern glittering with crystal. The floor is a grid of pressure plates.",
    PZ_REFLEX, "", "", 5 },
 /* 5 */ { "the Captain's Cache",
    "Behind a fake barrel, an iron strongbox is bolted to the floor.",
    PZ_LOCK, "", "", 8 },
 /* 6 */ { "the Frozen Shrine",
    "Under a drift of snow, a shrine of blue ice speaks in a voice like cracking frost.",
    PZ_RIDDLE, "\"I have keys but open no doors. I have space but no room. You can enter, but never leave. What am I?\"", "keyboard", 0 },
 /* 7 */ { "the Dragon's Door",
    "A door of black iron taller than a house, covered in scorched dials and sealed with a fiery lock.",
    PZ_LOCK, "", "", 12 },
 /* 8 */ { "the Shadow Gate",
    "Your shadow slides off the wall and stands in front of a tall gate. The gate flickers in a quick rhythm.",
    PZ_REFLEX, "", "", 9 },
 /* 9 */ { "the Hidden Archive",
    "A shelf of books slides aside. A single page glows: \"Answer, and the archive opens.\"",
    PZ_RIDDLE, "\"The more of me you take, the more you leave behind. What am I?\"", "step", 0 }
};

int secretFound(int id)
{
    return id >= 0 && id < SECRET_COUNT && adv.secrets[id];
}

const char *secretName(int id)
{
    return (id >= 0 && id < SECRET_COUNT) ? secrets[id].name : "a secret";
}

int secretsFoundCount(void)
{
    int n = 0;

    for (int i = 0; i < SECRET_COUNT; i++)
        n += adv.secrets[i];

    return n;
}

static void lowercase(char *text)
{
    for (; *text; text++)
        *text = (char)tolower((unsigned char)*text);
}

static int solveRiddle(const Secret *s)
{
    printf("%s\n", s->question);

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        char line[96];

        printf("Your answer (try %d of 3): ", attempt);
        readLine(line, sizeof line);
        lowercase(line);

        if (strstr(line, s->answer) != NULL)
        {
            printf("The riddle is answered.\n");
            return 1;
        }

        printf("Silence. That is not it.\n");
    }

    return 0;
}

static int solveLevers(const Secret *s)
{
    printf("%s\n", s->question);
    printf("Pull the levers in order by typing their letters (R, B, G), e.g. RBG.\n");

    for (int attempt = 1; attempt <= 3; attempt++)
    {
        char line[32];
        char order[8];
        int n = 0;

        printf("Order (try %d of 3): ", attempt);
        readLine(line, sizeof line);

        for (int i = 0; line[i] != '\0' && n < 6; i++)
            if (isalpha((unsigned char)line[i]))
                order[n++] = (char)tolower((unsigned char)line[i]);
        order[n] = '\0';

        if (strcmp(order, s->answer) == 0)
        {
            printf("CLUNK. The wall grinds aside!\n");
            return 1;
        }

        int right = 0;

        for (int i = 0; i < n && s->answer[i] != '\0'; i++)
            right += order[i] == s->answer[i];

        printf("The levers snap back. (%d in the right place)\n", right);
    }

    return 0;
}

static void secretReward(int id, int level)
{
    switch (id)
    {
    case 0:
        printf("The statue's mouth opens and hands you a bundle of old leather.\n");
        grantItem(8);
        player.gold += 60;
        printf("Behind it, lore carved in moss: \"The forest remembers everyone who listens.\" (+60 gold)\n");
        break;
    case 1:
        printf("Behind the wall, a hidden armoury!\n");
        grantItem(2);
        player.gold += 120;
        printf("+120 gold.\n");
        break;
    case 2:
        printf("The vault holds old relics.\n");
        grantMaterial(MAT_RUNE, 2);
        player.maxHealth += 10;
        player.health += 10;
        printf("A cool light settles over you: +10 maximum health, permanently.\n");
        break;
    case 3:
        printf("The lights fade and the cell opens. SIR GALAHAD is alive!\n");
        adv.flags[FLAG_GALAHAD_FOUND] = 1;
        player.gold += 100;
        printf("He thanks you with a weak smile. (+100 gold)\n");
        if (adv.affinity[NPC_GALAHAD] < 10)
            adv.affinity[NPC_GALAHAD] += 2;
        talkTo(NPC_GALAHAD);
        break;
    case 4:
        grantMaterial(MAT_CRYSTAL, 4);
        grantMaterial(MAT_SCALE, 1);
        break;
    case 5:
        player.gold += 200;
        grantMaterial(MAT_WOOD, 3);
        printf("+200 gold. A map in the box marks the lair of the PIRATE KING!\n");
        break;
    case 6:
        player.maxMana += 10;
        player.mana += 10;
        grantMaterial(MAT_CRYSTAL, 3);
        printf("The ice shrine pours its cold light into you: +10 maximum mana.\n");
        break;
    case 7:
        grantMaterial(MAT_SCALE, 2);
        printf("Beyond the door: heat, ash, and the sleeping lair of the ANCIENT DRAGON.\n");
        break;
    case 8:
        grantMaterial(MAT_SHADOW, 3);
        printf("The gate opens on a pit of shadows. The SHADOW BEAST waits below.\n");
        break;
    case 9:
        grantMaterial(MAT_RUNE, 2);
        player.skillPoints++;
        printf("The archive teaches you something: +1 skill point.\n");
        grantRelic(RELIC_BLACK_COMPASS);
        break;
    }

    (void)level;
}

void secretRoom(int dungeonLevel)
{
    int id = getLocation(dungeonLevel)->secret;

    if (id < 0 || adv.secrets[id])
    {
        printf("\nYou search the walls but find nothing new.\n");
        return;
    }

    const Secret *s = &secrets[id];

    if (player.relic == RELIC_BLACK_COMPASS)
        printf("\n(The Black Compass needle spins, then points steadily into the hidden room.)\n");

    printf("\n*** A SECRET ROOM: %s ***\n%s\n", s->name, s->intro);
    printf("1. Investigate\n2. Walk away\nEnter the number of your choice: ");

    if (readInt() != 1)
    {
        printf("You turn your back on it. It will still be here.\n");
        return;
    }

    int solved = 0;

    switch (s->type)
    {
    case PZ_RIDDLE: solved = solveRiddle(s); break;
    case PZ_LEVERS: solved = solveLevers(s); break;
    case PZ_LOCK:   solved = playLockpick(s->level, s->name) == MINI_WIN; break;
    case PZ_MEMORY: solved = playMemory(s->level, s->name) == MINI_WIN; break;
    case PZ_REFLEX: solved = playReflex(s->level, s->name) == MINI_WIN; break;
    }

    if (!solved)
    {
        printf("It stays sealed. Perhaps you can try again another time.\n");
        return;
    }

    adv.secrets[id] = 1;
    adv.stats[ST_SECRETS]++;
    guildOnSecret();

    printf("\nYou have discovered %s!\n", s->name);
    secretReward(id, dungeonLevel);
    achievementCheck();
}


/* =========================================================
   OPTIONAL BOSSES
   Not needed for the story, but each has two tricks and a unique
   reward. Most are behind a secret room.
   ========================================================= */

typedef struct
{
    const char *name;
    const char *intro;
    int level, hp, attack, defense;
    EnemySpecial first, second;
    int gold, xp;
} OptBoss;

static const OptBoss bosses[OPTBOSS_COUNT] =
{
 /* 0 */ { "The Ancient Dragon",
    "The mountain breathes. A dragon the size of a cathedral lifts its head from a bed of molten gold.",
    14, 480, 34, 12, SPECIAL_BURN, SPECIAL_RAGE, 500, 400 },
 /* 1 */ { "The Forgotten Knight",
    "A hollow suit of armour kneels before the empty throne, still holding an oath that no one remembers.",
    15, 380, 30, 16, SPECIAL_RAGE, SPECIAL_DOUBLE, 300, 350 },
 /* 2 */ { "The Shadow Beast",
    "The darkness stands up on four legs. It has too many eyes, and all of them are looking at you.",
    11, 300, 30, 8, SPECIAL_DRAIN, SPECIAL_DOUBLE, 250, 250 },
 /* 3 */ { "The Pirate King",
    "A towering pirate in a coat of gold lace rises from a throne of treasure chests. \"My treasure, my rules.\"",
    8, 230, 22, 8, SPECIAL_DOUBLE, SPECIAL_RAGE, 400, 200 },
 /* 4 */ { "The Elemental Guardian",
    "Fire, ice, earth and wind spiral together into a towering shape. It only opens its eyes for those who hold all three keys.",
    15, 420, 32, 12, SPECIAL_BURN, SPECIAL_REGEN, 300, 400 }
};

int optBossUnlocked(int id)
{
    switch (id)
    {
    case 0: return adv.secrets[7];
    case 1: return adv.flags[FLAG_GALAHAD_FOUND];
    case 2: return adv.secrets[8];
    case 3: return adv.secrets[5];
    case 4: return player.keys[0] && player.keys[1] && player.keys[2];
    }

    return 0;
}

int optBossesDefeatedCount(void)
{
    int n = 0;

    for (int i = 0; i < OPTBOSS_COUNT; i++)
        n += adv.optBoss[i];

    return n;
}

static void bossReward(int id)
{
    switch (id)
    {
    case 0:
        grantMaterial(MAT_SCALE, 5);
        printf("The dragon's hoard is yours. You earned the title DRAGON SLAYER.\n");
        grantRelic(RELIC_DRAGON_TOOTH);
        break;
    case 1:
        grantItem(14);
        printf("The knight finally rests. His sword remains.\n");
        grantRelic(RELIC_EYE_OF_ORACLE);
        break;
    case 2:
        grantMaterial(MAT_SHADOW, 5);
        grantRelic(RELIC_BLOODSTONE);
        break;
    case 3:
        grantItem(15);
        grantRelic(RELIC_KINGS_COIN);
        break;
    case 4:
        grantItem(13);
        grantMaterial(MAT_CRYSTAL, 4);
        grantMaterial(MAT_RUNE, 2);
        grantRelic(RELIC_FROZEN_HEART);
        break;
    }
}

void optionalBoss(int dungeonLevel)
{
    int id = getLocation(dungeonLevel)->optBoss;

    if (id < 0 || adv.optBoss[id])
        return;

    const OptBoss *b = &bosses[id];

    printf("\n*** OPTIONAL BOSS: %s ***\n%s\n", b->name, b->intro);
    printf("You could leave this alone. It is not needed to win the game.\n");
    printf("1. Challenge it\n2. Leave\nEnter the number of your choice: ");

    if (readInt() != 1)
    {
        printf("You back away quietly.\n");
        return;
    }

    Enemy boss;

    initializeEnemy(&boss, b->name, b->level, b->hp, b->attack, b->defense,
                    b->xp, b->gold, b->first);
    boss.special2 = b->second;

    if (startCombat(&player, &boss, 0) == 1)
    {
        adv.optBoss[id] = 1;
        guildOnOptBoss();
        bossReward(id);
        achievementCheck();
    }
}
