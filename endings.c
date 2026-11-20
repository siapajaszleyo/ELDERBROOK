#include <stdio.h>

#include "endings.h"
#include "adventure.h"
#include "npc.h"
#include "quest.h"
#include "secrets.h"
#include "achievements.h"
#include "guild.h"

#define LEGENDARY_SCORE 30
#define HEARTFELT_SCORE 14

/* The score. Every part of the journey counts: quests you finished,
   secrets you found, optional bosses you beat, achievements, and the
   friendships you made with the three knights. */
int endingScore(void)
{
    int friendships = adv.affinity[NPC_GALAHAD] + adv.affinity[NPC_MORGANA] + adv.affinity[NPC_BATMAN];

    return questsCompleted() * 2
         + secretsFoundCount()
         + optBossesDefeatedCount() * 2
         + achievementCount() / 2
         + friendships / 3;
}

static const char *endingTitle(void)
{
    int score = endingScore();

    if (score >= LEGENDARY_SCORE) return "THE LEGENDARY ENDING";
    if (score >= HEARTFELT_SCORE) return "THE HEARTFELT ENDING";
    return "THE QUIET ENDING";
}

static int knightNpc(void)
{
    switch (player.knightChosen)
    {
    case 1: return NPC_GALAHAD;
    case 2: return NPC_MORGANA;
    case 3: return NPC_BATMAN;
    }

    return -1;
}

void printEpilogue(const int *ids, int count)
{
    int score = endingScore();
    int knight = knightNpc();
    GuildId favored = GUILD_IRON_VOW;
    for (int g = 1; g < GUILD_COUNT; g++)
        if (adv.guildStanding[g] > adv.guildStanding[favored])
            favored = (GuildId)g;

    printf("\n=================================\n");
    printf("   %s\n", endingTitle());
    printf("=================================\n");

    /* The finale is bittersweet: the boss is down, but whoever marched
       with you paid for it. The tone shifts with how many stood beside
       you and how much of the world you touched along the way. */
    if (count > 0)
    {
        printf("The enemy is beaten, but the Citadel is quiet now. You walk back out\n");
        printf("past the fallen banners of the guilds who answered your call:\n");

        for (int i = 0; i < count; i++)
            printf("    - %s\n", guildName((GuildId)ids[i]));

        printf("\nEach banner answered for its own reasons. Their rivalries remain, and\n");
        printf("the price of the battle belongs to every faction that chose to come.\n\n");

        if (score >= LEGENDARY_SCORE)
            printf("When word reaches the town, no one cheers. Instead the whole kingdom\ncomes out to stand with you in silence — everyone you ever helped, there\nto share the weight of it.\n");
        else if (score >= HEARTFELT_SCORE)
            printf("Your friends gather at the tavern. There is no music tonight, only quiet\ncompany and a chair left empty for those who did not come home.\n");
        else
            printf("You watch the sunset alone from the town wall, carrying names the rest of\nthe world will never learn.\n");
    }
    else
    {
        printf("The enemy is beaten, and you did it with no banner but your own. There\n");
        printf("was no one to lose — and no one to turn to when it was done.\n\n");

        if (score >= LEGENDARY_SCORE)
            printf("The town gathers anyway. No guild stood with you, but your friends\nwill stand beside you for the rest of your days.\n");
        else if (score >= HEARTFELT_SCORE)
            printf("The tavern is warm and your friends are kind, and that is enough to come\nhome to.\n");
        else
            printf("You walk home alone under the stars, thinking about everything you have learned.\n");
    }

    /* Characters remember what you did for them */
    printf("\nAs a %s, you leave a mark shaped by your own discipline.\n",
           className((ClassKind)adv.playerClass));
    printf("The %s will remember your choices, though its rivals keep their own counsel.\n",
           guildName(favored));

    if (adv.questState[Q_HAMMER] == 2)
        printf("Brokk the blacksmith has forged a blade worthy of the fallen, and will not take a coin for it.\n");
    if (adv.questState[Q_FISHERMAN] == 2)
        printf("Old Pete pulls a gleaming fish from his bucket: \"For the feast. Or for you.\"\n");
    if (adv.questState[Q_TREASURE_MAP] == 2)
        printf("Zara sends a chest of treasure with a note: \"Spend it on something worth keeping.\"\n");
    if (adv.questState[Q_KNIGHT_RESCUE] == 2)
        printf("Sir Aldric weeps with pride. His student is safe, and so are you.\n");
    if (adv.flags[FLAG_WANDERER_TOKEN])
        printf("The Wanderer appears at the edge of the crowd and leaves a single page torn from the\nLibrary. On it, in your own handwriting, are two names, side by side.\n");
    if (adv.optBoss[0])
        printf("Songs are sung of the day you slew the Ancient Dragon. The bards leave out how nervous you were.\n");

    /* The knight you chose */
    if (knight >= 0)
    {
        if (adv.affinity[knight] >= 6)
            printf("%s stands beside you, prouder of this day than of any battle.\n", npcName(knight));
        else
            printf("%s watches from the back of the crowd and gives you a respectful nod.\n", npcName(knight));
    }

    /* The hidden best ending */
    if (score >= LEGENDARY_SCORE &&
        questsCompleted() == QUEST_COUNT &&
        optBossesDefeatedCount() == OPTBOSS_COUNT &&
        secretsFoundCount() == SECRET_COUNT &&
        achievementCount() == ACH_COUNT &&
        progress.dungeonsCleared == DUNGEON_COUNT)
    {
        printf("\n*** THE TRUE ENDING ***\n");
        printf("Every quest done, every secret found, every legend laid to rest. The sky\n");
        printf("fills with all four elements at once, dancing in celebration. Nobody in\n");
        printf("this world has ever seen anything like it, and no one will again.\n");
    }
}

void printSummary(void)
{
    printf("\n--------- Your adventure ---------\n");
    printf("Ending reached:      %s (score %d)\n", endingTitle(), endingScore());
    printf("Quests completed:    %d / %d\n", questsCompleted(), QUEST_COUNT);
    printf("Secret rooms found:  %d / %d\n", secretsFoundCount(), SECRET_COUNT);
    printf("Optional bosses:     %d / %d\n", optBossesDefeatedCount(), OPTBOSS_COUNT);
    printf("Achievements:        %d / %d\n", achievementCount(), ACH_COUNT);
    printf("Friendship (knights): Galahad %d, Morgana %d, Batman %d\n",
           adv.affinity[NPC_GALAHAD], adv.affinity[NPC_MORGANA], adv.affinity[NPC_BATMAN]);
}
