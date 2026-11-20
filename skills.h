#ifndef SKILLS_H
#define SKILLS_H

/* The skill tree: three paths of three skills. Learn them in order. */
enum
{
    SK_POWER = 0,   SK_CRIT,      SK_BERSERK,     /* Warrior  */
    SK_FIRE,        SK_LIGHTNING, SK_METEOR,      /* Mage     */
    SK_SHIELD,      SK_COUNTER,   SK_IRONWILL     /* Guardian */
};

const char *skillName(int skill);

/* Spend skill points (1 per level-up). Shown in the Training Area. */
void skillMenu(void);

#endif
