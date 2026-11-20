#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "guild.h"
#include "adventure.h"
#include "balance.h"
#include "input.h"
#include "monster.h"
#include "combat.h"
#include "world.h"

/* =========================================================
   GUILD DATA TABLES
   ========================================================= */

static const char *gNames[GUILD_COUNT] =
{
    "Order of the Iron Vow",
    "The Arcanum",
    "The Shrouded Hand",
    "The Wardens' Lodge",
    "The Covenant",
    "The Gilded Wheel"
};

static const char *gBlurb[GUILD_COUNT] =
{
    "Honour, protection, heavy steel — the lawful backbone of the realm.",
    "Regulated magic and knowledge; sworn rival of unlicensed sorcery.",
    "Thieves, smugglers and outlaws. No masters, only contracts.",
    "Hunters and trappers who keep the wild frontier in balance.",
    "Forbidden pacts and sacrifice — power at a terrible price.",
    "Luck, risk and information. Opportunists who trade with everyone."
};

/* rank 0 = Outsider, 1..6 = the ladder */
static const char *gRanks[GUILD_COUNT][7] =
{
    { "Outsider", "Recruit", "Squire", "Knight", "Knight-Captain", "Marshal", "Paladin" },
    { "Outsider", "Novice", "Adept", "Mage", "Magister", "Arch-Magister", "Archmage" },
    { "Outsider", "Footpad", "Cutpurse", "Rogue", "Shadow", "Nightmaster", "Shadowblade" },
    { "Outsider", "Tracker", "Forester", "Ranger", "Pathfinder", "Huntmaster", "Warden" },
    { "Outsider", "Supplicant", "Dabbler", "Warlock", "Pactbearer", "Soulbinder", "Dreadlord" },
    { "Outsider", "Punter", "Chancer", "Gambler", "Shark", "Whale", "High Roller" }
};

/* The per-rank perk blurbs (rank 1..6). */
static const char *gPerks[GUILD_COUNT][7] =
{
    { "",
      "buy Order armour recipes",
      "+5% defence in Order lands",
      "Oath of the Vanguard (start fights guarding)",
      "hire a man-at-arms for an expedition",
      "+5% vs. bandits & undead; command two allies",
      "+10% max HP; may call the Order in the finale" },
    { "",
      "buy staves & Magus robes",
      "-15% spell mana cost in the Spire's domain",
      "unlock a 4th spell slot",
      "free full mana restore once per expedition",
      "spells ignore 20% magic resistance",
      "+15% spell damage; may call the Arcanum in the finale" },
    { "",
      "access the black-market vendor",
      "+20% gold from all kills",
      "Smoke Exit (always escape one fight per trip)",
      "a fence buys anything at full value",
      "skip one lock minigame per expedition",
      "+10% crit; may call the Hand in the finale" },
    { "",
      "buy bows, arrows, trap kits",
      "reveal one secret room per location",
      "unlock a beast companion",
      "free fast-travel between cleared locations",
      "beast companion gets a second attack",
      "+1 trap effectiveness; may call the Lodge in the finale" },
    { "",
      "buy cursed reagents & the Bloodstone early",
      "convert gold -> Creds at a dark altar",
      "Soul Tithe (revive once per expedition)",
      "craftable relics unlocked",
      "sacrificed HP returns as a damage buff",
      "+1 relic slot; may call the Covenant in the finale" },
    { "",
      "raise table betting limits",
      "buy the King's Coin; re-roll one bad outcome",
      "a tip reveals the next boss weakness",
      "insurance — keep half your gold on death",
      "free re-roll on any table hand",
      "40% chance to double any payout; may call the Wheel" }
};

/* Creds dues to REACH rank 2..6 (index by target rank). */
static int duesForRank(int rank)
{
    switch (rank)
    {
    case 2: return GUILD_DUES_R2;
    case 3: return GUILD_DUES_R3;
    case 4: return GUILD_DUES_R4;
    case 5: return GUILD_DUES_R5;
    case 6: return GUILD_DUES_R6;
    default: return 0;
    }
}

/* Minimum standing tier to reach a given rank. */
static StandingTier tierForRank(int rank)
{
    if (rank >= 5) return STAND_ALLIED;
    if (rank >= 2) return STAND_FRIENDLY;
    return STAND_NEUTRAL;
}

/* Which ranks demand a proving trial (a sparring fight). */
static int rankHasTrial(int rank)
{
    return rank == 3 || rank == 5;
}

/* =========================================================
   IDENTITY / MAPPING
   ========================================================= */

const char *guildName(GuildId g)
{
    if (g < 0 || g >= GUILD_COUNT) return "Unknown Guild";
    return gNames[g];
}

const char *guildBlurb(GuildId g)
{
    if (g < 0 || g >= GUILD_COUNT) return "";
    return gBlurb[g];
}

const char *guildRankName(GuildId g, int rank)
{
    if (g < 0 || g >= GUILD_COUNT) return "Outsider";
    if (rank < 0 || rank > 6) rank = 0;
    return gRanks[g][rank];
}

GuildId homeGuildForClass(ClassKind c)
{
    switch (c)
    {
    case CLASS_KNIGHT:   return GUILD_IRON_VOW;
    case CLASS_ARCANIST: return GUILD_ARCANUM;
    case CLASS_ROGUE:    return GUILD_SHROUDED_HAND;
    case CLASS_RANGER:   return GUILD_WARDENS_LODGE;
    case CLASS_WARLOCK:  return GUILD_COVENANT;
    case CLASS_GAMBLER:  return GUILD_GILDED_WHEEL;
    default:             return GUILD_COUNT;
    }
}

/* World-Map homes (1..15 location ids from world.c). */
static const int gHomeLoc[GUILD_COUNT] =
{
    4,   /* Iron Vow      — The Castle of Ash     */
    10,  /* Arcanum       — The Ancient Library   */
    6,   /* Shrouded Hand — The Pirate's Den      */
    1,   /* Wardens'Lodge — The Whispering Forest */
    9,   /* Covenant      — The Shadow Realm      */
    3    /* Gilded Wheel  — The Sunken Temple      */
};

int guildHomeLocation(GuildId g)
{
    if (g < 0 || g >= GUILD_COUNT) return 0;
    return gHomeLoc[g];
}

GuildId guildAtLocation(int level)
{
    for (int g = 0; g < GUILD_COUNT; g++)
        if (gHomeLoc[g] == level)
            return (GuildId)g;
    return GUILD_COUNT;
}

/* =========================================================
   POLITICS
   ========================================================= */

/* Bloc membership: 0 = The Compact (law), 1 = The Freehold (outlaw),
   2 = unaligned (the Gilded Wheel). */
static int gBloc[GUILD_COUNT] = { 0, 0, 1, 0, 1, 2 };

static int isSchismPair(GuildId a, GuildId b)
{
    return (a == GUILD_ARCANUM && b == GUILD_COVENANT) ||
           (a == GUILD_COVENANT && b == GUILD_ARCANUM);
}

int guildAllyOf(GuildId g, GuildId other)
{
    if (g < 0 || g >= GUILD_COUNT || other < 0 || other >= GUILD_COUNT)
        return 0;
    if (g == other) return 0;
    if (gBloc[g] == 2 || gBloc[other] == 2) return 0; /* Wheel is flexible */
    return (gBloc[g] == gBloc[other]) ? 1 : -1;
}

StandingTier standingTierOf(int standing)
{
    if (standing >= GUILD_ALLIED_THRESHOLD)   return STAND_ALLIED;
    if (standing >= GUILD_FRIENDLY_THRESHOLD) return STAND_FRIENDLY;
    if (standing >  GUILD_WARY_THRESHOLD)     return STAND_NEUTRAL;
    if (standing >  GUILD_HOSTILE_THRESHOLD)  return STAND_WARY;
    return STAND_HOSTILE;
}

StandingTier guildTier(GuildId g)
{
    if (g < 0 || g >= GUILD_COUNT) return STAND_NEUTRAL;
    return standingTierOf(adv.guildStanding[g]);
}

const char *standingTierName(StandingTier t)
{
    switch (t)
    {
    case STAND_ALLIED:   return "Allied";
    case STAND_FRIENDLY: return "Friendly";
    case STAND_NEUTRAL:  return "Neutral";
    case STAND_WARY:     return "Wary";
    case STAND_HOSTILE:  return "Hostile";
    default:             return "Neutral";
    }
}

static void clampStanding(GuildId g)
{
    if (adv.guildStanding[g] > STANDING_MAX) adv.guildStanding[g] = STANDING_MAX;
    if (adv.guildStanding[g] < STANDING_MIN) adv.guildStanding[g] = STANDING_MIN;
}

void guildAddStanding(GuildId g, int n)
{
    if (g < 0 || g >= GUILD_COUNT || n == 0) return;

    adv.guildStanding[g] += n;
    clampStanding(g);

    for (int o = 0; o < GUILD_COUNT; o++)
    {
        if (o == (int)g) continue;

        int delta;
        if (isSchismPair(g, (GuildId)o))
            delta = -n;                       /* bitter Arcanum<->Covenant */
        else if (gBloc[g] == 2 || gBloc[o] == 2)
            delta = n / 4;                    /* Gilded Wheel stays flexible */
        else
            delta = (gBloc[g] == gBloc[o]) ? (n / 2) : (-n / 2);

        if (delta == 0) continue;
        adv.guildStanding[o] += delta;
        clampStanding((GuildId)o);
    }
}

/* =========================================================
   CREDS
   ========================================================= */

void guildAddCreds(GuildId g, int amount)
{
    if (g < 0 || g >= GUILD_COUNT || amount == 0) return;
    adv.guildCreds[g] += amount;
    if (adv.guildCreds[g] < 0)      adv.guildCreds[g] = 0;
    if (adv.guildCreds[g] > 999999) adv.guildCreds[g] = 999999;
}

void guildEarnHomeCreds(int amount)
{
    GuildId home = homeGuildForClass((ClassKind)adv.playerClass);
    if (home >= GUILD_COUNT) return;
    guildAddCreds(home, amount);
}

void guildOnQuestComplete(void)
{
    GuildId home = homeGuildForClass((ClassKind)adv.playerClass);
    if (home >= GUILD_COUNT) return;
    guildAddCreds(home, CREDS_PER_QUEST);
    guildAddStanding(home, STANDING_PER_QUEST_TIER / 3);
}

void guildOnOptBoss(void)
{
    GuildId home = homeGuildForClass((ClassKind)adv.playerClass);
    if (home >= GUILD_COUNT) return;
    guildAddCreds(home, CREDS_PER_OPTBOSS);
    guildAddStanding(home, STANDING_PER_OPTBOSS);
}

void guildOnSecret(void)
{
    GuildId home = homeGuildForClass((ClassKind)adv.playerClass);
    if (home >= GUILD_COUNT) return;
    guildAddCreds(home, CREDS_PER_SECRET);
    guildAddStanding(home, STANDING_PER_SECRET);
}

/* =========================================================
   SEEDING (called from applyClassLoadout)
   ========================================================= */

void guildSeedHome(ClassKind c)
{
    GuildId home = homeGuildForClass(c);
    if (home >= GUILD_COUNT) return;
    adv.guildRank[home]     = 1;
    adv.guildStanding[home] = STANDING_JOIN_SELF;
}

/* =========================================================
   QUESTLINES  (3 tiers, branching finale)
   ========================================================= */

static const char *qTitle[GUILD_COUNT] =
{
    "The Broken Vow", "The Forbidden Page", "The Last Score",
    "The Great Hunt", "The Hollow Bargain", "The House Edge"
};

static const char *qStep1[GUILD_COUNT] =
{
    "Recover a traitor knight's blade from the lower armoury.",
    "A page is missing from the Codex — begin tracing it.",
    "Case a sealed vault said to hold a fortune.",
    "Pick up the trail of a man-eater stalking the frontier.",
    "Make first contact with the Hollow King.",
    "Win a marked deck off a table cheat."
};

static const char *qStep2[GUILD_COUNT] =
{
    "Hunt the traitor into the Haunted Catacombs.",
    "The trail leads to a Covenant warlock who holds the page.",
    "Pull the heist in the Forgotten Kingdom with the crew.",
    "Corner the beast deep in the Serpent Caverns.",
    "Pay the price the Hollow King names.",
    "Run the rival table dry in the Sky Fortress."
};

static const char *qChoiceA[GUILD_COUNT] =
{
    "Execute the traitor by the Order's law.",
    "Return the page to the Arcanum.",
    "Split the take fairly with the crew.",
    "Take the trophy for the Lodge.",
    "Honour the pact and serve the Covenant.",
    "Keep the peace — buy the Wheel outright."
};

static const char *qChoiceB[GUILD_COUNT] =
{
    "Spare him — he flees to darker company.",
    "Keep the page for your own power.",
    "Betray the crew and take it all.",
    "Release the beast as sacred.",
    "Twist the bargain for personal power.",
    "Rig the house and pocket the lot."
};

/* Spawn and fight a themed proving opponent for a rank trial. */
static int guildTrial(GuildId g)
{
    Enemy e;
    int loc = guildHomeLocation(g);
    spawnMonster(&e, loc);

    /* Make it a proper proving fight: scale toward the hero. */
    e.level     = player.level;
    e.maxHealth += 30 + player.level * 4;
    e.health     = e.maxHealth;
    e.attack    += 3;
    snprintf(e.name, sizeof e.name, "%s Proving Champion", guildRankName(g, 1));

    printf("\nThe %s sets a proving trial before you.\n", guildName(g));
    printf("Defeat their champion to earn the rank.\n");

    int result = startCombat(&player, &e, 0);
    return result == 1;
}

static void guildQuestline(GuildId g)
{
    int stage = adv.guildQuestStage[g];

    printf("\n--- %s Questline: \"%s\" ---\n", guildName(g), qTitle[g]);

    if (stage >= 3)
    {
        printf("You have already seen \"%s\" through to its end.\n", qTitle[g]);
        return;
    }

    if (stage == 0)
    {
        printf("Tier I: %s\n", qStep1[g]);
        printf("You take up the task and see it done.\n");
        adv.guildQuestStage[g] = 1;
        guildAddCreds(g, CREDS_PER_QUEST);
        guildAddStanding(g, STANDING_PER_QUEST_TIER);
        printf("(+%d Creds, standing with %s rises.)\n", CREDS_PER_QUEST, guildName(g));
        return;
    }

    if (stage == 1)
    {
        printf("Tier II: %s\n", qStep2[g]);
        printf("The deed is harder, but you prevail.\n");
        adv.guildQuestStage[g] = 2;
        guildAddCreds(g, CREDS_PER_QUEST * 2);
        guildAddStanding(g, STANDING_PER_QUEST_TIER);
        printf("(+%d Creds, standing with %s rises.)\n", CREDS_PER_QUEST * 2, guildName(g));
        return;
    }

    /* stage == 2: the branching finale */
    printf("Tier III — the reckoning. Choose:\n");
    printf("1. %s\n", qChoiceA[g]);
    printf("2. %s\n", qChoiceB[g]);
    printf("Enter the number of your choice: ");

    int choice = readInt();
    adv.guildQuestStage[g] = 3;

    if (choice == 2)
    {
        printf("\nYou take the pragmatic road. Your purse swells, but "
               "loyalties strain.\n");
        guildAddCreds(g, CREDS_PER_QUEST * 4);
        guildAddStanding(g, -8);
        /* reward a touch of gold too */
        player.gold += 150;
        printf("(+%d Creds, +150 gold, %s standing dips.)\n",
               CREDS_PER_QUEST * 4, guildName(g));
    }
    else
    {
        printf("\nYou keep faith with the %s. They will remember it.\n", guildName(g));
        guildAddCreds(g, CREDS_PER_QUEST * 2);
        guildAddStanding(g, STANDING_PER_QUEST_TIER + 6);
        printf("(+%d Creds, %s standing climbs sharply.)\n",
               CREDS_PER_QUEST * 2, guildName(g));
    }
}

/* =========================================================
   RANK-UP & SERVICES
   ========================================================= */

static void guildRankUp(GuildId g)
{
    int current = adv.guildRank[g];

    if (current == 0)
    {
        /* Joining as an outsider. */
        if (guildTier(g) == STAND_HOSTILE || guildTier(g) == STAND_WARY)
        {
            printf("\nThe %s won't take you — your standing is too poor.\n", guildName(g));
            return;
        }
        printf("\nYou petition to join the %s.\n", guildName(g));
        if (player.gold < 50)
        {
            printf("The initiation fee is 50 gold — you can't afford it.\n");
            return;
        }
        player.gold -= 50;
        adv.guildRank[g] = 1;
        guildAddStanding(g, 10);
        printf("Welcome, %s. (Rank 1, -50 gold)\n", guildRankName(g, 1));
        return;
    }

    if (current >= 6)
    {
        printf("\nYou already hold the highest rank: %s.\n", guildRankName(g, 6));
        return;
    }

    int target = current + 1;
    int dues   = duesForRank(target);
    StandingTier need = tierForRank(target);

    printf("\nAdvance to %s:\n", guildRankName(g, target));
    printf("  Dues:     %d Creds (you have %d)\n", dues, adv.guildCreds[g]);
    printf("  Standing: %s or better (you are %s)\n",
           standingTierName(need), standingTierName(guildTier(g)));
    if (rankHasTrial(target))
        printf("  Trial:    a proving fight\n");
    printf("1. Attempt the promotion\n0. Not now\nEnter the number of your choice: ");

    if (readInt() != 1)
        return;

    if (adv.guildCreds[g] < dues)
    {
        printf("You lack the Creds for the dues.\n");
        return;
    }
    if (guildTier(g) < need)
    {
        printf("Your standing isn't high enough yet.\n");
        return;
    }
    if (rankHasTrial(target) && !guildTrial(g))
    {
        printf("You failed the trial. Return when you are ready.\n");
        return;
    }

    adv.guildCreds[g] -= dues;
    adv.guildRank[g]   = target;
    printf("\n*** You are promoted to %s of the %s! ***\n",
           guildRankName(g, target), guildName(g));
    printf("Perk unlocked: %s.\n", gPerks[g][target]);
}

static void guildServices(GuildId g)
{
    int rank = adv.guildRank[g];

    while (1)
    {
        printf("\n--- %s: Services (your rank: %s) ---\n",
               guildName(g), guildRankName(g, rank));
        printf("Creds: %d\n", adv.guildCreds[g]);
        printf("Perks you hold:\n");
        for (int r = 1; r <= rank; r++)
            printf("  %s: %s\n", guildRankName(g, r), gPerks[g][r]);
        printf("\nSpend Creds at the quartermaster:\n");
        printf("1. Health Potion (30 Creds)\n");
        printf("2. Mana Potion (25 Creds)\n");
        printf("3. Elixir, full heal in a bottle (90 Creds)\n");
        printf("0. Back\nEnter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            if (adv.guildCreds[g] >= 30) { adv.guildCreds[g] -= 30; player.potions++; printf("Bought a Health Potion.\n"); }
            else printf("Not enough Creds.\n");
            break;
        case 2:
            if (adv.guildCreds[g] >= 25) { adv.guildCreds[g] -= 25; player.manaPotions++; printf("Bought a Mana Potion.\n"); }
            else printf("Not enough Creds.\n");
            break;
        case 3:
            if (adv.guildCreds[g] >= 90) { adv.guildCreds[g] -= 90; player.elixirs++; printf("Bought an Elixir.\n"); }
            else printf("Not enough Creds.\n");
            break;
        case 0: return;
        default: printf("Invalid choice.\n");
        }
    }
}

static void guildPolitics(GuildId g)
{
    printf("\n--- %s: Standing & Politics ---\n", guildName(g));
    printf("%s\n", guildBlurb(g));
    printf("Your standing: %d (%s)\n",
           adv.guildStanding[g], standingTierName(guildTier(g)));
    printf("\nHow the %s sees the other guilds:\n", guildName(g));
    for (int o = 0; o < GUILD_COUNT; o++)
    {
        if (o == (int)g) continue;
        int rel = guildAllyOf(g, (GuildId)o);
        const char *tag = rel > 0 ? "ally" : rel < 0 ? "rival" : "neutral";
        if (isSchismPair(g, (GuildId)o)) tag = "bitter rival";
        printf("  %-22s %s\n", guildName((GuildId)o), tag);
    }
}

void guildVisit(int level)
{
    GuildId g = guildAtLocation(level);
    if (g >= GUILD_COUNT) return;

    if (guildTier(g) == STAND_HOSTILE)
    {
        printf("\nThe doors of the %s are barred to you. They want you gone.\n",
               guildName(g));
        return;
    }

    while (1)
    {
        printf("\n=================================\n");
        printf("  %s\n", guildName(g));
        printf("=================================\n");
        printf("%s\n", guildBlurb(g));
        if (adv.guildRank[g] > 0)
            printf("Member — Rank: %s   Creds: %d   Standing: %s\n",
                   guildRankName(g, adv.guildRank[g]), adv.guildCreds[g],
                   standingTierName(guildTier(g)));
        else
            printf("You are not a member here.   Standing: %s\n",
                   standingTierName(guildTier(g)));

        if (adv.guildRank[g] == 0)
            printf("\n1. Join the guild\n");
        else
            printf("\n1. Services\n");
        printf("2. Rank up\n");
        printf("3. Guild questline\n");
        printf("4. Standing & politics\n");
        printf("5. Leave\n");
        printf("Enter the number of your choice: ");

        switch (readInt())
        {
        case 1:
            if (adv.guildRank[g] == 0) guildRankUp(g);
            else                       guildServices(g);
            break;
        case 2: guildRankUp(g); break;
        case 3:
            if (adv.guildRank[g] == 0)
                printf("\nJoin the guild before taking its work.\n");
            else
                guildQuestline(g);
            break;
        case 4: guildPolitics(g); break;
        case 5: return;
        default: printf("Invalid choice.\n");
        }
    }
}

/* =========================================================
   THE GUILD LEDGER (pack sub-screen)
   ========================================================= */

void guildLedger(void)
{
    printf("\n=================================\n");
    printf("          GUILD LEDGER\n");
    printf("=================================\n");
    printf("%-22s %-13s %6s  %-9s\n", "Guild", "Rank", "Creds", "Standing");
    for (int g = 0; g < GUILD_COUNT; g++)
    {
        printf("%-22s %-13s %6d  %-9s\n",
               guildName((GuildId)g),
               adv.guildRank[g] ? guildRankName((GuildId)g, adv.guildRank[g]) : "—",
               adv.guildCreds[g],
               standingTierName(guildTier((GuildId)g)));
    }
    printf("\nBlocs: The Compact (Iron Vow, Arcanum, Wardens) vs. "
           "The Freehold (Shrouded Hand, Covenant).\n");
    printf("The Gilded Wheel trades with all. Arcanum and Covenant are "
           "bitter enemies.\n");
}

/* =========================================================
   HOSTILE AMBUSHES
   ========================================================= */

int guildAmbushCheck(int level)
{
    (void)level;

    int hostiles[GUILD_COUNT];
    int nh = 0;
    for (int g = 0; g < GUILD_COUNT; g++)
        if (guildTier((GuildId)g) == STAND_HOSTILE)
            hostiles[nh++] = g;

    if (nh == 0) return 1;

    int chance = nh * AMBUSH_BASE_CHANCE;
    if (chance > AMBUSH_MAX_CHANCE) chance = AMBUSH_MAX_CHANCE;
    if (rand() % 100 >= chance) return 1;

    GuildId g = (GuildId)hostiles[rand() % nh];

    Enemy e;
    spawnMonster(&e, guildHomeLocation(g));
    e.level     = player.level;
    e.maxHealth += 20 + player.level * 3;
    e.health     = e.maxHealth;
    e.attack    += 2;
    snprintf(e.name, sizeof e.name, "%s Enforcer", guildName(g));

    printf("\n!!! AMBUSH !!!\n");
    printf("Enforcers of the %s have been hunting you. They strike!\n", guildName(g));

    int result = startCombat(&player, &e, 1);

    if (result == 1)
    {
        guildAddStanding(g, AMBUSH_KILL_STANDING);
        printf("You cut down the %s enforcer — they will not forgive it.\n",
               guildName(g));
    }

    return result != -1;
}

/* =========================================================
   FINALE SUPPORT
   ========================================================= */

int guildFinaleAllies(int *ids)
{
    int blocStanding[2] = { 0, 0 };
    for (int g = 0; g < GUILD_COUNT - 1; g++)
        blocStanding[gBloc[g]] += adv.guildStanding[g];

    int chosenBloc = blocStanding[1] > blocStanding[0] ? 1 : 0;
    if (blocStanding[0] == blocStanding[1])
    {
        GuildId home = homeGuildForClass((ClassKind)adv.playerClass);
        if (home < GUILD_COUNT - 1)
            chosenBloc = gBloc[home];
    }

    int n = 0;
    for (int g = 0; g < GUILD_COUNT; g++)
        if (guildTier((GuildId)g) == STAND_ALLIED &&
            (g == GUILD_GILDED_WHEEL || gBloc[g] == chosenBloc))
            ids[n++] = g;
    return n;
}


