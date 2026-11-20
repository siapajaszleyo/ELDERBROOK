#include <stdio.h>

#include "npc.h"
#include "quest.h"
#include "adventure.h"
#include "achievements.h"
#include "input.h"
#include "guild.h"
#include "class.h"

static int isCasterClass(void)
{
    return adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK;
}

static void knightBondChoice(int npc)
{
    int affinity = adv.affinity[npc];
    if (affinity < 3)
    {
        if (npc == NPC_GALAHAD) printf("Galahad studies you carefully, still unsure of your purpose.\n");
        else if (npc == NPC_MORGANA) printf("Morgana smiles as if she knows the answer you have not asked.\n");
        else printf("Batman gives a terse nod and keeps watch on the road.\n");
        return;
    }

    if (npc == NPC_GALAHAD)
    {
        if (affinity < 7)
        {
            printf("Galahad offers to share his rations. 1. Share  2. Keep yours: ");
            if (readInt() == 1)
            {
                adv.affinity[npc]++;
                player.elixirs++;
                printf("You share a meal. Galahad gives you a full-heal elixir.\n");
            }
        }
        else if (affinity < 12)
        {
            printf("Galahad: the Wardens are stretched thin against the Shrouded Hand.\n1. Pledge a shift  2. Stay neutral: ");
            if (readInt() == 1)
            {
                guildAddStanding(GUILD_WARDENS_LODGE, 4);
                guildAddStanding(GUILD_SHROUDED_HAND, -2);
                player.defense += 6;
                adv.affinity[npc]++;
                printf("You gain +6 permanent defense and the Wardens' regard.\n");
            }
        }
        else if (!adv.vowReady && !adv.vowUsed)
        {
            adv.vowReady = 1;
            adv.affinity[npc]++;
            printf("Galahad's Vow: he will strike beside you for three turns in your next combat.\n");
        }
    }
    else if (npc == NPC_MORGANA)
    {
        if (affinity < 7)
        {
            printf("Morgana asks: which guild preserves knowledge? 1. Arcanum  2. Gilded Wheel: ");
            (void)readInt();
            if (adv.affinity[npc] < 30) adv.affinity[npc]++;
            printf("She accepts the answer with a knowing look.\n");
        }
        else if (affinity < 12)
        {
            printf("The Gilded Wheel is auctioning forbidden texts. 1. Help recover them  2. Let it go: ");
            if (readInt() == 1)
            {
                guildAddStanding(GUILD_ARCANUM, 4);
                guildAddStanding(GUILD_GILDED_WHEEL, -2);
                if (isCasterClass())
                {
                    player.maxMana += 10; player.mana += 10;
                    printf("Morgana shares the recovered lore: +10 permanent maximum MP.\n");
                }
                else { player.defense += 5; printf("Morgana gives you a ward: +5 permanent defense.\n"); }
                if (adv.affinity[npc] < 30) adv.affinity[npc]++;
            }
            else if (adv.affinity[npc] > 0) adv.affinity[npc]--;
        }
        else
        {
            adv.loanReady = 1;
            adv.loanMana = isCasterClass() ? 6 : 0;
            adv.loanShield = isCasterClass() ? 0 : 12;
            printf("Morgana's Loan: each combat this run begins with %s.\n",
                   isCasterClass() ? "6 bonus MP" : "a 12-point magic shield");
        }
    }
    else
    {
        if (affinity < 7)
        {
            printf("A patrol approaches. 1. Cover Batman  2. Step aside: ");
            if (readInt() == 1)
            {
                guildAddStanding(GUILD_SHROUDED_HAND, 2);
                guildAddStanding(GUILD_IRON_VOW, -2);
                if (adv.affinity[npc] < 30) adv.affinity[npc]++;
                printf("Batman remembers your choice.\n");
            }
            else printf("Batman notes your caution.\n");
        }
        else if (affinity < 12)
        {
            printf("Batman is hunting a target protected by the Iron Vow. 1. Join the hunt  2. Not my fight: ");
            if (readInt() == 1)
            {
                guildAddStanding(GUILD_SHROUDED_HAND, 4);
                guildAddStanding(GUILD_IRON_VOW, -2);
                grantOffense(&player, 8);
                if (adv.affinity[npc] < 30) adv.affinity[npc]++;
            }
        }
        else if (!adv.darkReady && !adv.darkUsed)
        {
            adv.darkReady = 1;
            printf("From the Dark: Batman will open your next combat with a guaranteed critical strike.\n");
        }
    }
}

typedef struct
{
    const char *name;
    const char *title;
    const char *intro;
} Npc;

static const Npc npcs[NPC_COUNT] =
{
    { "Brokk",       "the blacksmith",
      "A broad, soot-faced dwarf looks up from his anvil. \"Bring me materials and I'll bring you steel.\"" },
    { "Marla",       "the tavern keeper",
      "A woman with a quick smile polishes a mug. \"Everyone passes through here sooner or later. I hear everything.\"" },
    { "Tobias",      "the merchant",
      "A round man in a loud waistcoat spreads his hands. \"Potions, bombs, bargains! Your gold is always welcome.\"" },
    { "Sir Aldric",  "the old knight",
      "A grey-bearded knight leans on a notched sword. \"I trained three young knights once. I would give a great deal to see them safe.\"" },
    { "The Wanderer","a mysterious traveller",
      "A cloaked figure turns their head slowly. \"We were bound to meet. I know where you are going. I wonder if you do.\"" },
    { "Lucky Finn", "the gambler",
      "A man in a velvet coat shuffles cards one-handed. \"Care to lose a little money, friend?\"" },
    { "Seraphine",   "the mage",
      "A woman with star-dust in her hair studies you. \"Your aura is... interesting. Stay a while.\"" },
    { "Old Pete",    "the fisherman",
      "A weathered old man with a bamboo rod nods slowly. \"Patience, lad. The fish come to those who wait.\"" },
    { "Zara",        "the treasure hunter",
      "A woman with a coil of rope and a crooked grin tips her hat. \"Every locked door is an invitation.\"" },
    { "Sir Galahad", "the Pure",
      "A tall knight in spotless armour bows his head. \"I am in your debt. I will not forget it.\"" },
    { "Lady Morgana","the Wise",
      "A calm woman in a silver-blue robe looks at you carefully. \"I read people. You are... better than most.\"" },
    { "Sir Batman",  "the Brave",
      "A grinning knight slaps your shoulder. \"Finally, someone who can keep up! Let's make some noise!\"" }
};

const char *npcName(int npc)
{
    if (npc < 0 || npc >= NPC_COUNT)
        return "Someone";

    return npcs[npc].name;
}

int npcAffinity(int npc)
{
    return adv.affinity[npc];
}

int npcAvailable(int npc)
{
    int cleared = progress.dungeonsCleared;

    switch (npc)
    {
    case NPC_WANDERER:   return cleared >= 5;
    case NPC_FINN:       return cleared >= 2;
    case NPC_SERAPHINE:  return cleared >= 3;
    case NPC_ZARA:       return cleared >= 1;
    case NPC_BATMAN:     return cleared >= 2;
    case NPC_MORGANA:    return cleared >= 3;
    case NPC_GALAHAD:    return adv.flags[FLAG_GALAHAD_FOUND] || cleared >= 8;
    default:             return 1;
    }
}

/* What each character says depends on how far you have come: quests
   done, places cleared, secrets found. That is how they "remember". */
static const char *talkLine(int npc)
{
    int cleared = progress.dungeonsCleared;

    switch (npc)
    {
    case NPC_BROKK:
        if (adv.questState[Q_RARE_MATERIALS] == 2)
            return "Dragon scale! Wait until you see what the forge does with it. Check the crafting menu.";
        if (adv.questState[Q_HAMMER] == 2)
            return "That hammer rings true again. Crystal and iron make frost steel. Dragon scale makes legends.";
        if (cleared >= 6)
            return "You are bringing home better materials every trip. Bring dragon scales, and I'll make history.";
        return "Iron and wood make a fine sword. Bring me more and I'll show you the forge.";

    case NPC_MARLA:
        if (adv.optBoss[2])
            return "The Shadow Beast, dead! I'll tell that story till the day I die.";
        if (adv.secrets[5] && !adv.optBoss[3])
            return "They say a pirate cache hides a king of the sea. A king who dislikes visitors.";
        if (cleared >= 8)
            return "There's a door in the Dragon's Forge that no key opens. Find the hidden lever.";
        if (cleared >= 4)
            return "Strange cells and hidden walls in the old castle. Folk say a knight is still down there.";
        if (cleared >= 1)
            return "Everyone's talking about the strategist in the forest. He plays for keeps.";
        return "Welcome, traveller. Sit, drink. Many a hero starts right here.";

    case NPC_TOBIAS:
        if (player.gold >= 500)
            return "Look at that purse! You, my friend, are my best customer.";
        if (cleared >= 8)
            return "Dragon-forged goods are beyond me, but potions and bombs I do sell, always fresh.";
        return "A few coins for a potion could save your life. Think about it.";

    case NPC_ALDRIC:
        if (adv.questState[Q_KNIGHT_RESCUE] == 2)
            return "Galahad lives. That means the world feels a little lighter than it did yesterday.";
        if (cleared >= 4)
            return "The Castle of Ash... he went in and never came out. There must be a hidden room.";
        return "Knights, once, were more than armour. They were promises. Remember that.";

    case NPC_WANDERER:
        if (adv.flags[FLAG_WANDERER_TOKEN])
            return "You wear the token well. Not many see what is written on the last page.";
        if (cleared >= 10)
            return "The Library knows you now. Some books remember their readers.";
        return "Keys, doors, secrets. They all want to be opened. But in the right order.";

    case NPC_FINN:
        if (adv.flags[FLAG_GAMBLER_TITLE])
            return "They call you a real gambler now. Never let it go to your head.";
        if (adv.stats[ST_GAMBLE_WINS] >= 10)
            return "Ten wins? I wasn't counting. Okay, I was.";
        return "Dice, cards, any game you like. The house is generous. Sometimes.";

    case NPC_SERAPHINE:
        if (adv.flags[FLAG_ARTIFACT])
            return "The artifact still hums when you walk by. It likes you.";
        if (adv.materials[MAT_RUNE] >= 1)
            return "I sense an Ancient Rune in your pack. They don't come easily. Keep them close.";
        return "Magic is only memory that refuses to fade. Find me a rune and I'll show you.";

    case NPC_PETE:
        if (adv.stats[ST_LEGEND_FISH] > 0)
            return "You caught the Golden Koi?! I've chased that fish for fifty years...";
        if (adv.questState[Q_FISHERMAN] == 2)
            return "That rod will serve you well. Use it where the water runs deep.";
        return "Cast, wait, reel. Ease off when the line sings. The big ones fight, but they tire.";

    case NPC_ZARA:
        if (adv.questState[Q_TREASURE_MAP] == 2)
            return "Half the map was a decoy. Don't tell anyone. I'm still rich.";
        if (adv.stats[ST_CHESTS] >= 3)
            return "You open chests like you were born to it. Have you ever thought about joining me?";
        return "Locked chests are everywhere. Pick them carefully, or smash them and take your chances.";

    case NPC_GALAHAD:
        if (adv.optBoss[1])
            return "The forgotten knight is free, and so am I. My oath is fulfilled.";
        if (adv.affinity[NPC_GALAHAD] >= 6)
            return "I have fought beside many, but never in so good a cause as walking beside you.";
        return "Honour is a choice made every day. I try to choose well.";

    case NPC_MORGANA:
        if (adv.questState[Q_HERBS] == 2)
            return "My remedy saved a child's life. You have no idea what that means to me.";
        if (adv.affinity[NPC_MORGANA] >= 6)
            return "You are wiser than you pretend to be. I would trust my life to your judgement.";
        return "Watch what people do when they think no one is looking. That is who they are.";

    case NPC_BATMAN:
        if (adv.questState[Q_BRAVERY] == 2)
            return "You hit that bullseye like you were born for it! Brave? You're BRAVE!";
        if (adv.affinity[NPC_BATMAN] >= 6)
            return "You and me, side by side, there's nothing we can't take. Ha!";
        return "Fear is just excitement in a bad coat. Put on a better one!";
    }

    return "...";
}

void talkTo(int npc)
{
    printf("\n--- %s, %s ---\n", npcs[npc].name, npcs[npc].title);

    if (!adv.npcMet[npc])
    {
        adv.npcMet[npc] = 1;
        if (adv.affinity[npc] < 10)
            adv.affinity[npc]++;
        printf("%s\n", npcs[npc].intro);
    }

    if ((npc == NPC_GALAHAD && adv.affinity[NPC_BATMAN] >= 7) ||
        (npc == NPC_BATMAN && adv.affinity[NPC_GALAHAD] >= 7))
        printf("Their methods have left a fault line between them; neither yields.\n");

    while (1)
    {
        printf("\n1. Chat   2. Quests%s   3. Knight bond   4. Leave: ", questReadyFor(npc) ? " (a quest is ready to hand in!)" : "");

        int choice = readInt();

        if (choice == 1)
        {
            printf("\"%s\"\n", talkLine(npc));
            adv.stats[ST_NPC_TALKS]++;
        }
        else if (choice == 2)
        {
            if (questTurnInFor(npc) == 0 && !questOfferFor(npc))
                printf("\"Nothing for you right now. Come back when you have seen more.\"\n");
        }
        else if (choice == 3)
        {
            if (npc == NPC_GALAHAD || npc == NPC_MORGANA || npc == NPC_BATMAN)
                knightBondChoice(npc);
            else
                printf("You share a few words and continue on.\n");
        }
        else if (choice == 4)
        {
            achievementCheck();
            return;
        }
        else
            printf("Invalid choice.\n");
    }
}

/* Which character turns up in which location */
void npcEncounter(int dungeonLevel)
{
    static const int where[DUNGEON_COUNT] =
    {
        NPC_WANDERER, NPC_BATMAN,    NPC_MORGANA,  NPC_ALDRIC,   NPC_ZARA,
        NPC_FINN,     NPC_PETE,      NPC_BROKK,    NPC_WANDERER, NPC_SERAPHINE,
        NPC_ZARA,     NPC_BATMAN,    NPC_GALAHAD,  NPC_SERAPHINE, NPC_MORGANA
    };

    int npc = where[dungeonLevel - 1];

    if (!npcAvailable(npc))
        npc = NPC_WANDERER;

    printf("\nA familiar figure waits on the road ahead: %s!\n", npcs[npc].name);
    talkTo(npc);
}
