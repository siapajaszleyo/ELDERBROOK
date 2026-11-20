#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adventure.h"
#include "quest.h"
#include "achievements.h"
#include "balance.h"

Player    player;
Progress  progress;
Adventure adv;

void initializeAdventure(void)
{
    memset(&adv, 0, sizeof adv);

    adv.weapon = ITEM_RUSTY_SWORD;
    adv.owned[ITEM_RUSTY_SWORD] = 1;

    /* Character 2.0 defaults */
    adv.playerClass    = 0;   /* CLASS_NONE */
    adv.playerDiscipline = 0; /* DISC_NONE  */
    adv.relic          = 0;   /* RELIC_NONE */
    adv.difficulty     = 1;
}

const char *materialName(int material)
{
    static const char *names[MAT_COUNT] =
    {
        "Iron", "Wood", "Herb", "Crystal",
        "Dragon Scale", "Shadow Essence", "Ancient Rune"
    };

    if (material < 0 || material >= MAT_COUNT)
        return "Mystery Material";

    return names[material];
}

void grantMaterial(int material, int quantity)
{
    if (material < 0 || material >= MAT_COUNT || quantity <= 0)
        return;

    adv.materials[material] += quantity;

    if (adv.materials[material] > 999)
        adv.materials[material] = 999;

    printf("You collected %d x %s.\n", quantity, materialName(material));
}

void grantOffense(Player *target, int amount)
{
    if (target == NULL || amount <= 0)
        return;

    if (adv.playerClass == CLASS_ARCANIST || adv.playerClass == CLASS_WARLOCK)
    {
        target->maxMana += amount;
        target->mana += amount;
        if (target->mana > target->maxMana)
            target->mana = target->maxMana;
        printf("Your magical reserves deepen (+%d maximum mana).\n", amount);
    }
    else
    {
        target->attack += amount;
        printf("Your attack rises by %d.\n", amount);
    }
}

void onEnemyDefeated(const Enemy *enemy)
{
    adv.stats[ST_KILLS]++;

    /* Phase 3: a small bounty trickles to your home guild per kill. */
    guildEarnHomeCreds(GUILD_CREDS_PER_KILL);

    /* Monsters sometimes drop crafting materials */
    if (enemy->dropMaterial >= 0 && rand() % 100 < enemy->dropChance)
        grantMaterial(enemy->dropMaterial, 1);

    questNotify(QN_KILL, enemy->type);
    achievementCheck();
}
