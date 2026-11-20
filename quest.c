#include <stdio.h>

#include "quest.h"
#include "npc.h"
#include "adventure.h"
#include "items.h"
#include "monster.h"
#include "achievements.h"
#include "input.h"

enum { GK_KILL, GK_COLLECT, GK_SECRET, GK_OPTBOSS, GK_PROGRESS, GK_ARCHERY, GK_CLEAR };

typedef struct
{
    const char *name;
    int giver;
    const char *offer;
    const char *done;
    int goal, param, target;
    int minCleared;                 /* locations cleared before it is offered */
    /* rewards (0 or -1 = none) */
    int gold, xp, mat, matQty, item, skillPoint, hp, mp, atk, flag, rod, affinity;
} Quest;

/* THE QUESTS. To add one: add a row (and raise QUEST_COUNT in game.h). */
static const Quest quests[QUEST_COUNT] =
{
 /* 0 */ { "Rescue the Missing Knight", NPC_ALDRIC,
    "My student Sir Galahad vanished in the Castle of Ash. Find the hidden cell the old maps mention, and bring him home.",
    "Galahad is safe! You have my thanks, and this sword of the old order.",
    GK_SECRET, 3, 1, 2,   150, 80, -1, 0, 3, 0, 0, 0, 0, FLAG_GALAHAD_FOUND, 0, 3 },
 /* 1 */ { "The Stolen Hammer", NPC_BROKK,
    "Goblins stole my best hammer. Kill five of them and I'll find it in their pockets.",
    "My hammer! The forge sings again. Take this iron for your trouble.",
    GK_KILL, MT_GOBLIN, 5, 0,   100, 40, MAT_IRON, 5, -1, 0, 0, 0, 0, FLAG_HAMMER_BACK, 0, 2 },
 /* 2 */ { "The Lost Treasure", NPC_ZARA,
    "I've a map in pieces, and each piece is locked in a chest. Open three chests and the map is whole.",
    "The map is whole! The treasure is mine, and a share is yours.",
    GK_PROGRESS, QN_CHEST, 3, 1,   200, 60, MAT_RUNE, 2, -1, 0, 0, 0, 0, -1, 0, 2 },
 /* 3 */ { "Slay the Shadow Beast", NPC_MARLA,
    "Something huge stalks the Shadow Realm. Find where its gate is hidden and put it down.",
    "The Shadow Beast is dead! Drinks are on the house, forever. Take this armour.",
    GK_OPTBOSS, 2, 1, 6,   300, 150, -1, 0, 11, 0, 0, 0, 0, -1, 0, 2 },
 /* 4 */ { "Help the Fisherman", NPC_PETE,
    "Catch two rare fish for me. My old bones can't haul them in any more, and I'd like to see them again.",
    "Beautiful! Take my best rod: it finds the good fish far more often.",
    GK_PROGRESS, QN_FISH_RARE, 2, 0,   80, 30, -1, 0, -1, 0, 0, 0, 0, -1, 1, 2 },
 /* 5 */ { "The Ancient Artifact", NPC_SERAPHINE,
    "Bring me two Ancient Runes and I can wake the artifact I found. The knowledge in it is yours too.",
    "The artifact hums with power! Share it with me: you grow wiser.",
    GK_COLLECT, MAT_RUNE, 2, 3,   0, 100, -1, 0, -1, 1, 0, 10, 0, FLAG_ARTIFACT, 0, 2 },
 /* 6 */ { "The Gambling Tournament", NPC_FINN,
    "Win three blackjack hands with a bet of 30 gold or more. Then you can call yourself a gambler.",
    "Three big wins! You've got the nerve. The tavern will remember it.",
    GK_PROGRESS, QN_BJ_BIG_WIN, 3, 4,   250, 70, -1, 0, -1, 0, 0, 0, 0, FLAG_GAMBLER_TITLE, 0, 2 },
 /* 7 */ { "Rare Materials", NPC_BROKK,
    "Bring me two Dragon Scales. I'll teach you to forge dragon armour with them.",
    "Fine scales! Now I can teach you the old dragon-forging. Check the forge.",
    GK_COLLECT, MAT_SCALE, 2, 6,   0, 100, -1, 0, -1, 0, 0, 0, 0, FLAG_RECIPE_DRAGON, 0, 1 },
 /* 8 */ { "Herbs for the Wise", NPC_MORGANA,
    "I'm brewing a remedy and I'm six herbs short. Will you gather some for me?",
    "Just what I needed. You are kinder than you look. I'll teach you a little of my craft.",
    GK_COLLECT, MAT_HERB, 6, 3,   0, 60, -1, 0, -1, 1, 10, 0, 0, -1, 0, 3 },
 /* 9 */ { "A Test of Bravery", NPC_BATMAN,
    "A real hero never misses. Show me a score of 30 at an archery range and I'll call you brave.",
    "Thirty! Bravo! Take my battle-tips: you hit harder now.",
    GK_ARCHERY, 30, 1, 2,   100, 50, -1, 0, -1, 0, 0, 0, 3, -1, 0, 3 },
 /* 10 */ { "Oath of the Pure", NPC_GALAHAD,
    "A forgotten knight still guards the old kingdom, bound by a broken oath. Free him, and my own oath is lighter.",
    "You freed him. The oath is lifted, and my heart with it. Take my blessing.",
    GK_OPTBOSS, 1, 1, 12,   0, 160, -1, 0, -1, 0, 20, 0, 0, -1, 0, 3 },
 /* 11 */ { "The Traveller's Riddle", NPC_WANDERER,
    "I'll tell you a secret if you reach the great Library. The answer is waiting there for you.",
    "So you found it. Keep this token: it means more than you know yet.",
    GK_CLEAR, 10, 1, 5,   0, 120, MAT_RUNE, 3, -1, 0, 0, 0, 0, FLAG_WANDERER_TOKEN, 0, 2 },
 /* 12 */ { "Ash Gate Patrol", NPC_ALDRIC,
    "Clear the road through the Castle of Ash. The Iron Vow needs a safe patrol route.", "The patrol route is open again.",
    GK_CLEAR, 4, 1, 3, 90, 40, MAT_IRON, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 13 */ { "Iron for the Oath", NPC_BROKK,
    "Bring eight Iron for the Vow's damaged shields.", "The shields are repaired and ready.",
    GK_COLLECT, MAT_IRON, 8, 2, 100, 40, MAT_WOOD, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 14 */ { "Restless Dead", NPC_ALDRIC,
    "Put four skeletons to rest before they reach the old road.", "The road is quiet for now.",
    GK_KILL, MT_SKELETON, 4, 2, 110, 50, -1, 0, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 15 */ { "The Uncatalogued Rune", NPC_SERAPHINE,
    "Find one Ancient Rune so the Arcanum can catalogue its origin.", "The rune's script has been recorded.",
    GK_COLLECT, MAT_RUNE, 1, 5, 120, 60, MAT_CRYSTAL, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 16 */ { "A Door Too Many", NPC_ZARA,
    "Open four locked chests. The Arcanum needs their seals intact.", "Every seal has been copied.",
    GK_PROGRESS, QN_CHEST, 4, 4, 130, 60, -1, 0, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 17 */ { "The Quiet Stacks", NPC_WANDERER,
    "Reach the Ancient Library and listen for what the shelves remember.", "The library has given up another of its stories.",
    GK_CLEAR, 10, 1, 7, 120, 70, MAT_RUNE, 1, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 18 */ { "Cutlass Count", NPC_FINN,
    "The Shrouded Hand wants five pirates cleared from its smuggling lanes.", "The lanes are clear, and the Hand owes you.",
    GK_KILL, MT_PIRATE, 5, 4, 150, 60, MAT_WOOD, 3, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 19 */ { "A Better Table", NPC_FINN,
    "Win two large blackjack hands; the Hand wants to know who owns the table.", "The dealer has learned to respect your face.",
    GK_PROGRESS, QN_BJ_BIG_WIN, 2, 5, 150, 70, -1, 0, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 20 */ { "Into the Shadow", NPC_MARLA,
    "Reach the Shadow Realm and learn what the Hand is hunting there.", "The target has been found and the trail marked.",
    GK_CLEAR, 9, 1, 7, 160, 80, MAT_SHADOW, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 21 */ { "Winter's Catch", NPC_PETE,
    "Bring back two rare fish from the Frozen Wastes.", "Those fish will feed the whole watch.",
    GK_PROGRESS, QN_FISH_RARE, 2, 5, 130, 60, MAT_HERB, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 22 */ { "Wolves at the Tree Line", NPC_GALAHAD,
    "Drive three frostwolves away from the northern paths.", "The Wardens can keep watch without being hunted.",
    GK_KILL, MT_FROSTWOLF, 3, 6, 160, 80, MAT_SCALE, 1, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 23 */ { "Signal Above the Clouds", NPC_ZARA,
    "Reach the Sky Fortress and restore a signal visible from the forest.", "The signal burns bright above the clouds.",
    GK_CLEAR, 12, 1, 9, 180, 90, MAT_WOOD, 4, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 24 */ { "A Measured Shadow", NPC_WANDERER,
    "Bring three Shadow Essences to the Covenant's hidden altar.", "The altar accepts the essence without a word.",
    GK_COLLECT, MAT_SHADOW, 3, 7, 180, 90, MAT_RUNE, 1, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 25 */ { "Night Terrors", NPC_MORGANA,
    "Defeat three shadows before they slip through the old wards.", "The wards hold through another night.",
    GK_KILL, MT_SHADOW, 3, 8, 200, 100, MAT_SHADOW, 1, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 26 */ { "The Fourfold Seal", NPC_SERAPHINE,
    "Reach the Realm of Elements and study its fourfold seal.", "The seal's pattern is now in safe hands.",
    GK_CLEAR, 14, 1, 11, 220, 110, MAT_CRYSTAL, 3, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 27 */ { "A Mark at Distance", NPC_BATMAN,
    "Score at least 35 at an archery range to prove your aim.", "That shot will be remembered.",
    GK_ARCHERY, 35, 1, 5, 150, 70, -1, 0, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 28 */ { "Prism Components", NPC_BROKK,
    "Gather five crystals for the Gilded Wheel's prize display.", "The display is ready for the next tournament.",
    GK_COLLECT, MAT_CRYSTAL, 5, 7, 190, 90, MAT_IRON, 2, -1, 0, 0, 0, 0, -1, 0, 1 },
 /* 29 */ { "The Last Wager", NPC_FINN,
    "Reach the Final Citadel and bring back proof that the wager is settled.", "The Wheel pays its debt, though the game is not forgotten.",
    GK_CLEAR, 15, 1, 13, 250, 120, MAT_RUNE, 2, -1, 0, 0, 0, 0, -1, 0, 1 }
};

static const int questFaction[QUEST_COUNT] = {
    [Q_VOW_PATROL] = GUILD_IRON_VOW, [Q_VOW_IRON] = GUILD_IRON_VOW,
    [Q_VOW_UNDEAD] = GUILD_IRON_VOW, [Q_ARCANUM_RUNES] = GUILD_ARCANUM,
    [Q_ARCANUM_LOCKS] = GUILD_ARCANUM, [Q_ARCANUM_LIBRARY] = GUILD_ARCANUM,
    [Q_HAND_PIRATES] = GUILD_SHROUDED_HAND, [Q_HAND_TOURNEY] = GUILD_SHROUDED_HAND,
    [Q_HAND_SHADOW] = GUILD_SHROUDED_HAND, [Q_WARDENS_FISH] = GUILD_WARDENS_LODGE,
    [Q_WARDENS_WOLVES] = GUILD_WARDENS_LODGE, [Q_WARDENS_SKY] = GUILD_WARDENS_LODGE,
    [Q_COVENANT_ESSENCE] = GUILD_COVENANT, [Q_COVENANT_SHADOWS] = GUILD_COVENANT,
    [Q_COVENANT_PRISM] = GUILD_COVENANT, [Q_WHEEL_ARCHERY] = GUILD_GILDED_WHEEL,
    [Q_WHEEL_CRYSTALS] = GUILD_GILDED_WHEEL, [Q_WHEEL_CITADEL] = GUILD_GILDED_WHEEL
};

static int progressOf(int q)
{
    const Quest *quest = &quests[q];
    int value;

    switch (quest->goal)
    {
    case GK_COLLECT:  value = adv.materials[quest->param]; break;
    case GK_SECRET:   value = adv.secrets[quest->param]; break;
    case GK_OPTBOSS:  value = adv.optBoss[quest->param]; break;
    case GK_ARCHERY:  value = adv.stats[ST_ARCHERY_BEST] >= quest->param; break;
    case GK_CLEAR:    value = progress.dungeonsCleared >= quest->param; break;
    default:          value = adv.questProgress[q]; break;
    }

    return value > quest->target ? quest->target : value;
}

static int isReady(int q)
{
    return adv.questState[q] == 1 && progressOf(q) >= quests[q].target;
}

void questNotify(int kind, int param)
{
    for (int q = 0; q < QUEST_COUNT; q++)
    {
        if (adv.questState[q] != 1)
            continue;

        if ((quests[q].goal == GK_KILL && kind == QN_KILL && quests[q].param == param) ||
            (quests[q].goal == GK_PROGRESS && quests[q].param == kind))
        {
            if (adv.questProgress[q] < quests[q].target)
                adv.questProgress[q]++;
        }
    }
}

int questReadyFor(int npc)
{
    for (int q = 0; q < QUEST_COUNT; q++)
        if (quests[q].giver == npc && isReady(q))
            return 1;

    return 0;
}

static void reward(int q)
{
    const Quest *r = &quests[q];

    printf("\n*** QUEST COMPLETE: %s ***\n\"%s\"\n", r->name, r->done);

    if (r->goal == GK_COLLECT)
        adv.materials[r->param] -= r->target;           /* handed over */

    if (r->gold > 0)
    {
        printf("Reward: %d gold.\n", r->gold);
        player.gold += r->gold;
    }
    if (r->mat >= 0 && r->matQty > 0)
        grantMaterial(r->mat, r->matQty);
    if (r->item > 0)
        grantItem(r->item);
    if (r->skillPoint > 0)
    {
        player.skillPoints += r->skillPoint;
        printf("Reward: %d skill point.\n", r->skillPoint);
    }
    if (r->hp > 0)
    {
        player.maxHealth += r->hp;
        player.health += r->hp;
        printf("Reward: +%d maximum health, permanently.\n", r->hp);
    }
    if (r->mp > 0)
    {
        player.maxMana += r->mp;
        player.mana += r->mp;
        printf("Reward: +%d maximum mana, permanently.\n", r->mp);
    }
    if (r->atk > 0)
    {
        grantOffense(&player, r->atk);
        printf("Reward: permanent offense improvement.\n");
    }
    if (r->flag >= 0)
        adv.flags[r->flag] = 1;
    if (r->rod > 0)
    {
        adv.rodLevel += r->rod;
        printf("Reward: a better fishing rod!\n");
    }

    adv.questState[q] = 2;

    if (q >= Q_VOW_PATROL)
    {
        guildAddStanding((GuildId)questFaction[q], 4);
        printf("Faction standing rises with %s.\n", guildName((GuildId)questFaction[q]));
    }

    /* Phase 3: finishing a quest earns Creds & standing with your guild. */
    guildOnQuestComplete();

    if (r->affinity > 0 && adv.affinity[r->giver] < 10)
    {
        adv.affinity[r->giver] += r->affinity;
        if (adv.affinity[r->giver] > 10)
            adv.affinity[r->giver] = 10;
        printf("%s likes you more.\n", npcName(r->giver));
    }

    if (r->xp > 0)
        gainXP(&player, r->xp);

    achievementCheck();
}

int questTurnInFor(int npc)
{
    int done = 0;

    for (int q = 0; q < QUEST_COUNT; q++)
        if (quests[q].giver == npc && isReady(q))
        {
            reward(q);
            done++;
        }

    return done;
}

int questOfferFor(int npc)
{
    for (int q = 0; q < QUEST_COUNT; q++)
    {
        if (quests[q].giver != npc || adv.questState[q] != 0 ||
            progress.dungeonsCleared < quests[q].minCleared)
            continue;

        printf("\n[Quest offered] %s\n\"%s\"\n", quests[q].name, quests[q].offer);
        printf("1. Accept   2. Not now: ");

        if (readInt() == 1)
        {
            adv.questState[q] = 1;
            adv.questProgress[q] = 0;
            printf("Quest accepted: %s\n", quests[q].name);
        }
        else
            printf("Perhaps later.\n");

        return 1;
    }

    return 0;
}

void questJournal(void)
{
    int shown = 0;

    printf("\n=================================\n");
    printf("         QUEST JOURNAL\n");
    printf("=================================\n");

    for (int q = 0; q < QUEST_COUNT; q++)
    {
        if (adv.questState[q] == 1)
        {
            printf("[ACTIVE] %s (from %s): %d/%d%s\n", quests[q].name,
                   npcName(quests[q].giver), progressOf(q), quests[q].target,
                   isReady(q) ? "   READY TO HAND IN" : "");
            shown++;
        }
    }
    for (int q = 0; q < QUEST_COUNT; q++)
    {
        if (adv.questState[q] == 2)
        {
            printf("[DONE]   %s\n", quests[q].name);
            shown++;
        }
    }

    if (shown == 0)
        printf("No quests yet. Talk to the people of the town!\n");
}

int questsCompleted(void)
{
    int n = 0;

    for (int q = 0; q < QUEST_COUNT; q++)
        n += adv.questState[q] == 2;

    return n;
}

int questKnightsFriendly(void)
{
    return adv.affinity[NPC_GALAHAD] >= 3 &&
           adv.affinity[NPC_MORGANA] >= 3 &&
           adv.affinity[NPC_BATMAN] >= 3;
}
