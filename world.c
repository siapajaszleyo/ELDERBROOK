#include <stdlib.h>

#include "world.h"
#include "monster.h"
#include "events.h"
#include "adventure.h"
#include "secrets.h"
#include "class.h"
#include "dungeonmod.h"

/* THE WORLD. One row per location. To change what a place feels like,
   change its row: which monsters live there, which events are common,
   what the miniboss is, and what the story scene says. */
static const Location locations[DUNGEON_COUNT] =
{
 { /* 1 */ "The Whispering Forest",
   "Tall trees murmur overhead. Something in the leaves seems to know your name.",
   "%s slinks out from between the ancient trees!",
   { MT_SLIME, MT_GOBLIN }, 2,
   { EV_FORAGE, EV_FORAGE, EV_ITEM, EV_FISHING, EV_CHEST, EV_NPC }, 6,
   MG_CHESS, "Grand Strategist Orlen",
   "A hooded strategist sits on a mossy stump, a chessboard carved into it.",
   "A fox with silver eyes watches you from a hollow log, a small pouch at its feet.",
   "Take the pouch gently and leave a coin", "Snatch the pouch and run",
   0, -1 },

 { /* 2 */ "The Ruined Village",
   "Broken roofs and cold hearths. The village was abandoned in a hurry.",
   "%s shambles out of a collapsed house!",
   { MT_GOBLIN, MT_SKELETON }, 2,
   { EV_MERCHANT, EV_ITEM, EV_CHEST, EV_NPC, EV_FORAGE }, 5,
   MG_SNAKE, "Serpent Queen Vyra",
   "Coils of scales slide through the old well. A crowned serpent rises from it.",
   "A child's wooden toy lies in the road beside a locked cellar door.",
   "Leave the toy where it is and pick the cellar lock carefully", "Smash the cellar door open",
   1, -1 },

 { /* 3 */ "The Sunken Temple",
   "Water drips from carved ceilings. Every footstep echoes twice.",
   "%s rises dripping from the flooded halls!",
   { MT_SLIME, MT_SKELETON, MT_SERPENT }, 3,
   { EV_FISHING, EV_FISHING, EV_CHEST, EV_DICE_TABLE, EV_ITEM }, 5,
   MG_DICE, "Gambler Kael",
   "A dice-tower of black glass stands in the flooded atrium. Kael grins behind it.",
   "A golden idol glows on a pedestal above a pool of black water.",
   "Study the pedestal for traps before touching anything", "Grab the idol at once",
   2, -1 },

 { /* 4 */ "The Castle of Ash",
   "Grey snow that is not snow falls on blackened battlements.",
   "%s strides out of the smouldering gatehouse!",
   { MT_SKELETON, MT_ORC, MT_IMP }, 3,
   { EV_MERCHANT, EV_ARCHERY, EV_CHEST, EV_MINE, EV_NPC }, 5,
   MG_CHESS, "Warden Ashgrave",
   "A giant chessboard is burned into the throne room floor. The Warden waits on it.",
   "A knight's sword is driven into the floor of the great hall, still warm.",
   "Examine the sword and the scorch marks around it", "Wrench the sword free",
   3, -1 },

 { /* 5 */ "The Serpent Caverns",
   "The tunnels twist like a living thing. The walls glitter with scales.",
   "%s lunges from a side tunnel!",
   { MT_SERPENT, MT_SLIME, MT_ORC }, 3,
   { EV_MINE, EV_MINE, EV_FORAGE, EV_CHEST, EV_ITEM }, 5,
   MG_SNAKE, "Naga Empress Sissra",
   "The tunnel opens into a vast nest. The Empress coils around a heap of treasure.",
   "A crystal vein pulses with light in a narrow crack in the wall.",
   "Chip at the crystal slowly and carefully", "Blast the vein open at once",
   4, -1 },

 { /* 6 */ "The Pirate's Den",
   "Lanterns sway over a hidden cove. Someone is singing a very rude song.",
   "%s swaggers out of the shadows, cutlass drawn!",
   { MT_PIRATE, MT_GOBLIN, MT_SKELETON }, 3,
   { EV_BLACKJACK, EV_BLACKJACK, EV_DICE_TABLE, EV_MERCHANT, EV_FISHING, EV_CHEST }, 6,
   MG_BLACKJACK, "Captain Redtide",
   "Captain Redtide deals cards on a barrel top. \"Twenty-one or nothing, landlubber.\"",
   "A drunken pirate offers to sell you a treasure map for a few coins.",
   "Pay his price and read the map carefully", "Steal the map while he sways",
   5, 3 },

 { /* 7 */ "The Frozen Wastes",
   "A whiteout of wind and ice. Your breath freezes before it leaves your lips.",
   "%s howls out of the blizzard!",
   { MT_FROSTWOLF, MT_ICEGOLEM, MT_SKELETON }, 3,
   { EV_FISHING, EV_MINE, EV_FORAGE, EV_ITEM, EV_NPC }, 5,
   MG_MEMORY, "Frost Oracle Ysolde",
   "An ice-sculpted oracle floats above a frozen lake, lights dancing around her.",
   "A frozen traveller sits beside a dead fire, a small chest in his lap.",
   "Light the fire again and warm the traveller", "Pry the chest from his frozen hands",
   6, -1 },

 { /* 8 */ "The Dragon's Forge",
   "Rivers of fire run through the black rock. The air rings with distant hammers.",
   "%s crawls out of the molten slag!",
   { MT_IMP, MT_ORC, MT_DRAKE }, 3,
   { EV_MINE, EV_MINE, EV_ARCHERY, EV_MERCHANT, EV_CHEST }, 5,
   MG_REFLEX, "Forgemaster Ignar",
   "Forgemaster Ignar swings a hammer of living flame. \"Match my rhythm, mortal.\"",
   "A cooling blade rests on an anvil, its edge glowing faintly.",
   "Wait for it to cool and take it with tongs", "Grab it with your bare hand",
   7, 0 },

 { /* 9 */ "The Shadow Realm",
   "Colour drains from everything. Your shadow moves a heartbeat too late.",
   "%s tears itself loose from the darkness!",
   { MT_SHADOW, MT_WRAITH, MT_IMP }, 3,
   { EV_CHEST, EV_ITEM, EV_MINE, EV_NPC, EV_DICE_TABLE }, 5,
   MG_CHESS, "Shade Sovereign",
   "The Sovereign's chessboard is made of light and shadow, and the pieces move on their own.",
   "Your shadow stops following you and points at a crack in the ground.",
   "Follow where the shadow points", "Stamp on the shadow and walk on",
   8, 2 },

 { /* 10 */ "The Ancient Library",
   "Endless shelves of whispering books. A quill writes your name in the air.",
   "%s slides out from between the stacks!",
   { MT_WRAITH, MT_SKELETON, MT_BONEKNIGHT }, 3,
   { EV_CHEST, EV_NPC, EV_ITEM, EV_ITEM, EV_MERCHANT }, 5,
   MG_LOCKPICK, "The Locksmith Wraith",
   "The Locksmith Wraith holds a thousand keys. \"Open my door, if you can.\"",
   "A book lies open on a lectern, glowing on a page about you.",
   "Read the page slowly and copy it down", "Tear the page out and take it",
   9, -1 },

 { /* 11 */ "The Haunted Catacombs",
   "Bones line the walls in neat patterns. Something is counting your steps.",
   "%s claws its way out of a burial niche!",
   { MT_SKELETON, MT_WRAITH, MT_BONEKNIGHT }, 3,
   { EV_CHEST, EV_MINE, EV_ITEM, EV_DICE_TABLE, EV_FORAGE }, 5,
   MG_SNAKE, "Bone Serpent Morrow",
   "A skeletal serpent as long as the hall uncoils from a pile of skulls.",
   "A tomb lid has been pushed open from the inside. A hand holds out a ring.",
   "Say a prayer for the dead and take the ring", "Take the ring and slam the lid",
   -1, -1 },

 { /* 12 */ "The Sky Fortress",
   "Clouds drift through the broken floor. The wind never stops.",
   "%s dives out of the open sky!",
   { MT_WYVERN, MT_BONEKNIGHT, MT_ORC }, 3,
   { EV_ARCHERY, EV_ARCHERY, EV_MERCHANT, EV_ITEM, EV_CHEST }, 5,
   MG_ARCHERY, "Sky Marshal Aeris",
   "Marshal Aeris stands on a bridge of wind, a banner on a far tower for a target.",
   "A broken ballista sits among scattered bolts and a flapping flag.",
   "Repair the ballista with care", "Fire it as it is",
   -1, -1 },

 { /* 13 */ "The Forgotten Kingdom",
   "A grand city, lost to time. Its streets still hold memories of music.",
   "%s steps out from a crumbling archway!",
   { MT_BONEKNIGHT, MT_WRAITH, MT_DRAKE }, 3,
   { EV_MERCHANT, EV_NPC, EV_CHEST, EV_BLACKJACK, EV_ITEM }, 5,
   MG_DICE, "The Forgotten Jester",
   "A skeletal jester juggles ivory dice before an empty throne.",
   "A throne stands empty, a crown resting on the seat.",
   "Bow to the throne before touching the crown", "Put the crown on",
   -1, 1 },

 { /* 14 */ "The Realm of Elements",
   "Fire, water, earth and air swirl in sheets. The ground changes colour as you walk.",
   "%s coalesces out of the raging elements!",
   { MT_ELEMENTAL, MT_DRAKE, MT_ICEGOLEM, MT_IMP }, 4,
   { EV_MINE, EV_MINE, EV_CHEST, EV_FORAGE, EV_ITEM }, 5,
   MG_MEMORY, "Prism Warden Aether",
   "The Prism Warden flashes four colours in a pattern, a challenge older than language.",
   "Four glowing pedestals hum in a circle, each a different colour.",
   "Listen to each pedestal before touching any", "Slam your hand on the brightest one",
   -1, 4 },

 { /* 15 */ "The Final Citadel",
   "Towering black walls. At the top, something ancient waits for you.",
   "%s guards the citadel stairs!",
   { MT_BONEKNIGHT, MT_DRAKE, MT_ELEMENTAL, MT_SHADOW }, 4,
   { EV_MERCHANT, EV_CHEST, EV_ITEM, EV_NPC, EV_FORAGE }, 5,
   MG_CHESS, "Dark Chancellor Vorn",
   "The Chancellor stands at a board of obsidian. \"Every game ends here.\"",
   "A long staircase climbs the tower. Halfway up, a small door stands ajar.",
   "Check the door for traps before stepping in", "Burst through the door",
   -1, -1 }
};

const Location *getLocation(int level)
{
    if (level < 1)
        level = 1;
    if (level > DUNGEON_COUNT)
        level = DUNGEON_COUNT;

    return &locations[level - 1];
}


/* ---------------------------------------------------------
   PLANNING AN EXPEDITION
   Story scene first, then a shuffled mix of fights and other events,
   then the miniboss. We avoid three fights in a row so the rhythm is
   fight, explore, choose, play, reward, never just fight, fight, fight.
   --------------------------------------------------------- */

static int tooManyFightsInARow(const int *plan, int start, int end)
{
    for (int i = start + 2; i < end; i++)
        if (plan[i] == EV_MONSTER &&
            plan[i - 1] == EV_MONSTER &&
            plan[i - 2] == EV_MONSTER)
            return 1;

    return 0;
}

static void insertEvent(int *plan, int *count, int position, int event)
{
    for (int i = *count; i > position; i--)
        plan[i] = plan[i - 1];

    plan[position] = event;
    (*count)++;
}

void buildPlanFor(Progress *g, int level, int revisit)
{
    const Location *where = getLocation(level);
    int count = 0;

    g->revisit = revisit;

    if (!revisit)
        g->plan[count++] = EV_STORY;

    int bodyStart = count;
    int monsters = 2 + rand() % 2;                  /* at least 2 fights */
    int extras = 2 + rand() % 2;

    /* Phase 5b: the per-run modifier reshapes the plan. */
    monsters += modMonsterDelta();
    extras   += modExtraDelta();
    if (monsters < 1) monsters = 1;
    if (extras   < 0) extras   = 0;

    for (int i = 0; i < monsters; i++)
        g->plan[count++] = EV_MONSTER;

    for (int i = 0; i < extras; i++)
    {
        int event;
        int guard = 0;

        /* Only events that exist in this build can be chosen */
        do
        {
            event = where->extras[rand() % where->extraCount];
        }
        while (!eventIsReady(event) && ++guard < 20);

        if (eventIsReady(event))
            g->plan[count++] = event;
        else
            g->plan[count++] = EV_ITEM;
    }

    /* Shuffle the body (Fisher-Yates), retrying if it came out as a
       long run of fights */
    for (int attempt = 0; attempt < 30; attempt++)
    {
        for (int i = count - 1; i > bodyStart; i--)
        {
            int j = bodyStart + rand() % (i - bodyStart + 1);
            int temp = g->plan[i];

            g->plan[i] = g->plan[j];
            g->plan[j] = temp;
        }

        if (!tooManyFightsInARow(g->plan, bodyStart, count))
            break;
    }

    /* Optional content: this location's secret room (if not found yet),
       and its optional boss once the way to it is open. */
    if (where->secret >= 0 && !adv.secrets[where->secret] && rand() % 100 < (65 + relicSecretBonus()))
        insertEvent(g->plan, &count, bodyStart + rand() % (count - bodyStart + 1), EV_SECRET);

    if (where->optBoss >= 0 && !adv.optBoss[where->optBoss] &&
        optBossUnlocked(where->optBoss) && rand() % 100 < 70)
        insertEvent(g->plan, &count, bodyStart + rand() % (count - bodyStart + 1), EV_OPTBOSS);

    /* Phase 5b: dungeon-variety rooms.
       A guaranteed elite from the modifier; hazard rooms from Haunted /
       Flooded runs; and, on ordinary runs too, an occasional shrine or
       branching fork so no two expeditions feel quite the same. */
    if (modForcesElite() && count < MAX_PLAN - 1)
        insertEvent(g->plan, &count, bodyStart + rand() % (count - bodyStart + 1), EV_ELITE);

    {
        int hazards = modHazardBias();
        if (hazards == 0 && rand() % 100 < 20)
            hazards = 1;                        /* a light base chance */

        for (int h = 0; h < hazards && count < MAX_PLAN - 1; h++)
            insertEvent(g->plan, &count,
                        bodyStart + rand() % (count - bodyStart + 1), EV_HAZARD);
    }

    if (count < MAX_PLAN - 1 && rand() % 100 < 35)
        insertEvent(g->plan, &count, bodyStart + rand() % (count - bodyStart + 1), EV_SHRINE);

    if (count < MAX_PLAN - 1 && rand() % 100 < 30)
        insertEvent(g->plan, &count, bodyStart + rand() % (count - bodyStart + 1), EV_BRANCH);

    /* An ordinary run may still promote one trash fight to an elite. */
    if (!modForcesElite() && count < MAX_PLAN - 1 && rand() % 100 < 18)
    {
        for (int i = bodyStart; i < count; i++)
            if (g->plan[i] == EV_MONSTER)
            {
                g->plan[i] = EV_ELITE;
                break;
            }
    }

    if (!revisit)
        g->plan[count++] = EV_MINIBOSS;

    g->planCount = count;
    g->planIndex = 0;
}
