#include <stdio.h>

#include "skills.h"
#include "adventure.h"
#include "input.h"

typedef struct
{
    const char *name;
    const char *path;
    const char *description;
} Skill;

static const Skill skills[SKILL_COUNT] =
{
    { "Power Strike",    "Warrior",  "Combat action: a heavy blow, 80% stronger than a slash (8 mana)." },
    { "Critical Hit",    "Warrior",  "Passive: +15% chance to land a critical hit (double damage)." },
    { "Berserker",       "Warrior",  "Passive: +50% damage while your health is below 40%." },
    { "Fire Mastery",    "Mage",     "Passive: Fireball deals +12 damage." },
    { "Lightning",       "Mage",     "Combat action: a bolt that ignores armour (12 mana)." },
    { "Meteor",          "Mage",     "Combat action: a devastating strike (30 mana)." },
    { "Shield",          "Guardian", "Combat action: halve the next hit you take (6 mana)." },
    { "Counterattack",   "Guardian", "Passive: 30% chance to strike back when hit." },
    { "Iron Will",       "Guardian", "Passive: once per fight, survive a killing blow with 1 health." }
};

const char *skillName(int skill)
{
    if (skill < 0 || skill >= SKILL_COUNT)
        return "?";

    return skills[skill].name;
}

/* New characters commit to one of the three persistent skill branches. */
static int selectedPath(void)
{
    int selected = -1;
    for (int i = 0; i < SKILL_COUNT; i++)
        if (player.skills[i])
        {
            int path = i / 3;
            if (selected >= 0 && selected != path)
                return -2; /* legacy save with cross-path skills */
            selected = path;
        }
    return selected;
}

/* A skill is open when its branch is selected and its prerequisite is known. */
static int prerequisiteMet(int skill)
{
    int chosen = selectedPath();
    if (chosen >= 0 && chosen != skill / 3)
        return 0;
    return skill % 3 == 0 || player.skills[skill - 1];
}

void skillMenu(void)
{
    while (1)
    {
        printf("\n=================================\n");
        printf("          SKILL TREE\n");
        printf("=================================\n");
        printf("Skill points: %d   (you earn one every level)\n\n", player.skillPoints);
        int chosen = selectedPath();
        if (chosen >= 0)
            printf("Branch committed: %s. Other branches are sealed for this character.\n\n",
                   skills[chosen * 3].path);

        for (int i = 0; i < SKILL_COUNT; i++)
        {
            const char *state = player.skills[i] ? "[learned]"
                              : prerequisiteMet(i) ? "[available]"
                              : "[locked]";

            if (i % 3 == 0)
                printf("-- %s path --\n", skills[i].path);

            printf("%d. %-14s %-11s %s\n", i + 1, skills[i].name, state, skills[i].description);
        }

        printf("\nEnter a number to learn that skill, or 0 to leave: ");

        int choice = readInt();

        if (choice == 0)
            return;

        if (choice < 1 || choice > SKILL_COUNT)
        {
            printf("Invalid choice.\n");
            continue;
        }

        int skill = choice - 1;

        if (player.skills[skill])
            printf("You already know %s.\n", skills[skill].name);
        else if (!prerequisiteMet(skill))
        {
            if (selectedPath() >= 0 && selectedPath() != skill / 3)
                printf("This character has committed to another branch.\n");
            else
                printf("Learn %s first.\n", skills[skill - 1].name);
        }
        else if (player.skillPoints < 1)
            printf("You have no skill points.\n");
        else
        {
            player.skillPoints--;
            player.skills[skill] = 1;
            printf("You learned %s!\n", skills[skill].name);
        }
    }
}
