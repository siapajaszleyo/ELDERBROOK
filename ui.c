#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "adventure.h"
#include "class.h"

#define UI_WIDTH 54

void uiRule(void)
{
    putchar('+');
    for (int i = 0; i < UI_WIDTH; i++) putchar('-');
    puts("+");
}

void uiTitle(const char *title)
{
    int length = 0;
    while (title[length] != '\0' && length < UI_WIDTH - 2) length++;
    int left = (UI_WIDTH - length) / 2;
    int right = UI_WIDTH - length - left;
    putchar('|');
    for (int i = 0; i < left; i++) putchar(' ');
    printf("%.*s", length, title);
    for (int i = 0; i < right; i++) putchar(' ');
    puts("|");
}

void uiPanelLine(const char *text)
{
    printf("| %-*.*s |\n", UI_WIDTH - 2, UI_WIDTH - 2, text);
}

void showHealthBar(void)
{
    int shown = player.health < 0 ? 0 : player.health;
    int hearts = shown * 10 / player.maxHealth;

    printf("Health: [");
    for (int i = 0; i < hearts; i++)
        printf(HEART);
    for (int i = hearts; i < 10; i++)
        printf(" ");
    printf("] %d/%d\n", shown, player.maxHealth);
}

void showManaMeter(void)
{
    int shown = player.mana < 0 ? 0 : player.mana;
    int gems = player.maxMana > 0 ? shown * 10 / player.maxMana : 0;

    printf("Mana:   [");
    for (int i = 0; i < gems; i++)
        printf(GEM);
    for (int i = gems; i < 10; i++)
        printf(" ");
    printf("] %d/%d\n", shown, player.maxMana);
}

void showKeys(void)
{
    const char *names[NUM_KEYS] = {"Fire", "Earth", "Air"};

    printf("Keys:   ");
    for (int i = 0; i < NUM_KEYS; i++)
        printf("%s %s  ", names[i], player.keys[i] ? "[X]" : "[ ]");
    printf("\n");
}

void showStats(void)
{
    char line[160];
    putchar('\n');
    uiRule();
    snprintf(line, sizeof line, "%s | Level %d %s | XP %d/%d",
             player.name, player.level, rankName(player.level),
             player.xp, player.xpToNextLevel);
    uiTitle(line);
    uiRule();
    showHealthBar();
    showManaMeter();
    snprintf(line, sizeof line, "Gold: %d doubloons | Attack: %d | Defense: %d",
             player.gold, player.attack, player.defense);
    uiPanelLine(line);
    snprintf(line, sizeof line, "Health potions: %d | Mana potions: %d",
             player.potions, player.manaPotions);
    uiPanelLine(line);
    if (adv.playerClass != CLASS_NONE)
    {
        snprintf(line, sizeof line, "Class: %s", className((ClassKind)adv.playerClass));
        if (adv.playerDiscipline != DISC_NONE)
        {
            size_t used = strlen(line);
            snprintf(line + used, sizeof line - used, " / %s",
                     disciplineName((Discipline)adv.playerDiscipline));
        }
        uiPanelLine(line);
    }
    if (adv.relic != RELIC_NONE)
    {
        snprintf(line, sizeof line, "Relic: %s", relicName((RelicKind)adv.relic));
        uiPanelLine(line);
    }
    showKeys();
    uiRule();
}
