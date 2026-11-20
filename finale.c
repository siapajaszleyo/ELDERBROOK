#include <stdio.h>

#include "finale.h"
#include "adventure.h"
#include "guild.h"
#include "player.h"
#include "combat.h"

/* =========================================================
   PER-GUILD FINALE BEATS
   Each marched guild gets three lines: how it arrives, what its
   boon feels like, and how it falls. Indexed by GuildId.
   ========================================================= */

typedef struct
{
    const char *arrival;
    const char *boon;
    const char *fall;
} FinaleBeat;

static const FinaleBeat beats[GUILD_COUNT] =
{
    /* GUILD_IRON_VOW */
    { "A column of the Order of the Iron Vow marches up the causeway, shields locked.",
      "They form a wall at your back — you feel unbreakable (shield of extra health).",
      "The Iron Vow holds the breach to the last, and the shield wall goes still." },

    /* GUILD_ARCANUM */
    { "Mages of the Arcanum arrive in a shimmer of wards and quiet counting.",
      "They weave a counter-sigil that smothers the boss's deadliest trick.",
      "The last ward shatters; the Arcanum's casters fall where they stand." },

    /* GUILD_SHROUDED_HAND */
    { "The Shrouded Hand was already inside — figures peel from the shadows to join you.",
      "They mark every seam in the boss's guard (your strikes land far truer).",
      "Blades flash in the dark, then fewer shadows remain — the Hand is spent." },

    /* GUILD_WARDENS_LODGE */
    { "Rangers of the Wardens' Lodge take the high ledges, arrows nocked.",
      "Volley after volley staggers the boss and drives it back.",
      "Their quivers run dry; the Wardens fall covering your advance." },

    /* GUILD_COVENANT */
    { "The Covenant comes last, trailing cold fire and whispered pacts.",
      "They pour borrowed life into your arms — power at a terrible price (your blows turn savage).",
      "The pact collects its due, and the Covenant's warlocks crumble to ash." },

    /* GUILD_GILDED_WHEEL */
    { "The Gilded Wheel saunters in, already betting on how this ends.",
      "They stake everything on you — fortune bends your way (luck and edge surge).",
      "The dice come up wrong at last, and the Wheel's gamblers fall laughing." }
};


static void applyPlayerBoon(GuildId g)
{
    switch (g)
    {
    case GUILD_IRON_VOW:
    {
        int shield = player.maxHealth * 30 / 100;   /* +30% max HP wall */
        player.maxHealth += shield;
        player.health    += shield;
        break;
    }
    case GUILD_SHROUDED_HAND:
        player.critChance += 25;                     /* truer strikes    */
        break;
    case GUILD_COVENANT:
        grantOffense(&player, 15);                  /* savage blows     */
        break;
    case GUILD_GILDED_WHEEL:
        grantOffense(&player, 10);                  /* luck and edge    */
        player.critChance += 10;
        break;
    case GUILD_ARCANUM:
    case GUILD_WARDENS_LODGE:
        /* these two act on the boss, not the hero (see finaleApplyToBoss) */
        break;
    default:
        break;
    }

    if (player.critChance > 100)
        player.critChance = 100;
}


int finaleMuster(int *ids)
{
    int count = guildFinaleAllies(ids);

    printf("\n=================================\n");
    printf("     THE MUSTER AT THE CITADEL\n");
    printf("=================================\n");

    if (count == 0)
    {
        printf("No banners but your own fly at the gate. No guild answered your\n");
        printf("cause, and you will face what waits inside alone.\n");
        return 0;
    }

    printf("The guilds that chose to answer each arrive under their own banners:\n\n");

    for (int i = 0; i < count; i++)
    {
        GuildId g = (GuildId)ids[i];

        printf("- %s\n", guildName(g));
        printf("    %s\n", beats[g].arrival);
        printf("    %s\n", beats[g].boon);

        applyPlayerBoon(g);
    }

    printf("\nWith allies at your side, you stride through the Citadel gate.\n");

    return count;
}


void finaleApplyToBoss(Enemy *boss, const int *ids, int count)
{
    if (boss == NULL)
        return;

    for (int i = 0; i < count; i++)
    {
        GuildId g = (GuildId)ids[i];

        if (g == GUILD_ARCANUM)
        {
            /* ward away the boss's deadliest trick, and blunt it a little */
            boss->special2 = SPECIAL_NONE;
            boss->attack -= 4;
        }
        else if (g == GUILD_WARDENS_LODGE)
        {
            /* staggering volleys: weaker and already wounded */
            boss->attack -= 6;
            boss->health -= 40;
            boss->maxHealth -= 40;
        }
    }

    if (boss->attack < 1)
        boss->attack = 1;
    if (boss->maxHealth < 1)
        boss->maxHealth = 1;
    if (boss->health < 1)
        boss->health = 1;
}


void finaleFalls(const int *ids, int count)
{
    if (count == 0)
    {
        printf("\nNo one stood beside you — and no one fell. The last blow is yours,\n");
        printf("struck alone in the ringing silence of the Citadel.\n");
        return;
    }

    printf("\n--- The last stand ---\n");
    printf("As the enemy gathers for one final, terrible effort, your allies spend\n");
    printf("themselves to buy you the opening:\n\n");

    for (int i = 0; i < count; i++)
    {
        GuildId g = (GuildId)ids[i];
        printf("    %s\n", beats[g].fall);
    }

    printf("\nThe way is clear, bought with their lives. You step forward alone and\n");
    printf("land the final blow.\n");
}
