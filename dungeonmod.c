#include <stdio.h>
#include <stdlib.h>

#include "dungeonmod.h"
#include "guild.h"

/* Transient per-run state. Deliberately NOT part of the save file. */
static DungeonMod g_currentMod     = DMOD_NONE;
static GuildId    g_contestedGuild = GUILD_COUNT;   /* valid only for CONTESTED */

/* Collect the guilds you have driven Hostile. Returns how many, and
   fills ids[] (size >= GUILD_COUNT). */
static int hostileGuilds(int *ids)
{
    int n = 0;

    for (int g = 0; g < GUILD_COUNT; g++)
        if (guildTier((GuildId)g) == STAND_HOSTILE)
            ids[n++] = g;

    return n;
}

DungeonMod rollDungeonMod(int level)
{
    (void)level;

    g_contestedGuild = GUILD_COUNT;

    /* Roughly half of runs stay plain. */
    if (rand() % 100 < 50)
    {
        g_currentMod = DMOD_NONE;
        return g_currentMod;
    }

    /* Build the menu of modifiers available right now. */
    DungeonMod menu[DMOD_COUNT];
    int count = 0;

    menu[count++] = DMOD_HAUNTED;
    menu[count++] = DMOD_FLOODED;
    menu[count++] = DMOD_INFESTED;
    menu[count++] = DMOD_CALM;

    int hostiles[GUILD_COUNT];
    int hostileCount = hostileGuilds(hostiles);
    if (hostileCount > 0)
        menu[count++] = DMOD_CONTESTED;

    g_currentMod = menu[rand() % count];

    if (g_currentMod == DMOD_CONTESTED)
        g_contestedGuild = (GuildId)hostiles[rand() % hostileCount];

    return g_currentMod;
}

void setDungeonMod(DungeonMod m)
{
    g_currentMod = m;
    if (m != DMOD_CONTESTED)
        g_contestedGuild = GUILD_COUNT;
}

DungeonMod currentDungeonMod(void)
{
    return g_currentMod;
}

const char *dungeonModName(DungeonMod m)
{
    switch (m)
    {
    case DMOD_HAUNTED:   return "Haunted";
    case DMOD_FLOODED:   return "Flooded";
    case DMOD_CONTESTED: return "Guild-Contested";
    case DMOD_INFESTED:  return "Infested";
    case DMOD_CALM:      return "Calm";
    case DMOD_NONE:      return "Ordinary";
    default:             return "Ordinary";
    }
}

void announceDungeonMod(void)
{
    switch (g_currentMod)
    {
    case DMOD_HAUNTED:
        printf("\n[The air is cold and wrong here — this place is HAUNTED. Expect\n");
        printf(" more hazards, but the dead guard richer treasures.]\n");
        break;
    case DMOD_FLOODED:
        printf("\n[Dark water pools in every hall — this run is FLOODED. The footing\n");
        printf(" is treacherous and the way is full of hazards.]\n");
        break;
    case DMOD_CONTESTED:
        printf("\n[Enforcers of the %s have seized this place — it is GUILD-CONTESTED.\n",
               guildName(g_contestedGuild));
        printf(" Expect a tougher fight, but driving them out will mend your standing.]\n");
        break;
    case DMOD_INFESTED:
        printf("\n[Something has bred down here without end — this run is INFESTED.\n");
        printf(" It runs long, with more fights and a monstrous elite.]\n");
        break;
    case DMOD_CALM:
        printf("\n[For once, all is still — this run is CALM. Shorter and safer, but\n");
        printf(" the pickings are lean.]\n");
        break;
    case DMOD_NONE:
    default:
        break;
    }
}

int modMonsterDelta(void)
{
    switch (g_currentMod)
    {
    case DMOD_INFESTED:  return +1;
    case DMOD_CONTESTED: return +1;
    case DMOD_CALM:      return -1;
    default:             return 0;
    }
}

int modExtraDelta(void)
{
    switch (g_currentMod)
    {
    case DMOD_INFESTED: return +1;
    case DMOD_CALM:     return -1;
    default:            return 0;
    }
}

int modForcesElite(void)
{
    return (g_currentMod == DMOD_INFESTED || g_currentMod == DMOD_CONTESTED) ? 1 : 0;
}

int modHazardBias(void)
{
    return (g_currentMod == DMOD_HAUNTED || g_currentMod == DMOD_FLOODED) ? 1 : 0;
}

void dungeonModOnClear(int level)
{
    (void)level;

    if (g_currentMod == DMOD_CONTESTED && g_contestedGuild != GUILD_COUNT)
    {
        /* You cleared the turf war: the ousted guild grudgingly warms to you. */
        guildAddStanding(g_contestedGuild, 15);
        printf("\nWith the %s driven out, word spreads that you cleared their claim.\n",
               guildName(g_contestedGuild));
        printf("Your standing with them improves.\n");
    }
}
