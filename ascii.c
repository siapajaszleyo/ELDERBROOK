#include <stdio.h>
#include <string.h>
#include "ascii.h"

void asciiClear(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void asciiRule(int w)
{
    if (w < 8)  w = 8;
    if (w > 78) w = 78;
    printf("  +");
    for (int i = 0; i < w; i++)
        putchar('-');
    printf("+\n");
}

void asciiBanner(const char *title)
{
    int len = (int)strlen(title);
    int pad = (60 - len) / 2;
    if (pad < 0) pad = 0;

    printf("\n");
    asciiRule(60);
    printf("  |");
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s", title);
    for (int i = 0; i < 60 - pad - len; i++) putchar(' ');
    printf("|\n");
    asciiRule(60);
}

void asciiStatusBox(const char *line)
{
    printf("  [%s]\n", line);
}

void asciiTitleScreen(void)
{
    printf("\n");
    printf("      .  *  .        +     .        *     .      +   .\n");
    printf("         .      *               .            *\n");
    printf("              /\\   /\\      *          /\\   /\\\n");
    printf("    *        /  \\_/  \\       .      /  \\_/  \\      *\n");
    printf("            /         \\           /         \\\n");
    printf("           /   |^^^|   \\    +    /   |^^^|   \\\n");
    printf("          |    |   |    |       |    |   |    |\n");
    printf("          |   /     \\   |  *    |   /     \\   |\n");
    printf("          |  |  [ ]  |  |       |  |  [ ]  |  |\n");
    printf("          |  |_______|  |       |  |_______|  |\n");
    printf("          |_____________|       |_____________|\n");
    printf("               ||                     ||\n");
    printf("    ~~~~~~~~~~~||~~~~~~~~~~~~~~~~~~~~~||~~~~~~~~~~~\n");
    printf("\n");
    printf("      __________________________________________________\n");
    printf("     /                                                  \\\n");
    printf("    |                                                    |\n");
    printf("    |     _______  _______ _________ _______             |\n");
    printf("    |    (  ____ \\(  ____ )\\__   __/(  ____ \\            |\n");
    printf("    |    | (    \\/| (    )|   ) (   | (    \\/            |\n");
    printf("    |    | (__    | (____)|   | |   | |                  |\n");
    printf("    |    |  __)   |  _____)   | |   | |                  |\n");
    printf("    |    | (      | (         | |   | |                  |\n");
    printf("    |    | (____/\\| )      ___) (___| (____/\\            |\n");
    printf("    |    (_______/|/       \\_______/(_______/            |\n");
    printf("    |                                                    |\n");
    printf("    |            R P G   A D V E N T U R E               |\n");
    printf("    |                Expanded Edition                    |\n");
    printf("    |                                                    |\n");
    printf("    |   fifteen dungeons  ·  six guilds  ·  one path     |\n");
    printf("    |                                                    |\n");
    printf("     \\__________________________________________________/\n");
    printf("\n");
    printf("              \\\\                 //\n");
    printf("               \\\\   _......_   //\n");
    printf("                \\\\_/  o  o  \\_//\n");
    printf("                 (  \\  __  /  )\n");
    printf("                  \\  '----'  /          a dragon sleeps\n");
    printf("                 / `--------' \\         beyond the walls\n");
    printf("                /__/  |  |  \\__\\\n");
    printf("                   |  |  |  |\n");
    printf("                   |__|  |__|\n");
    printf("\n");
    printf("         .    *         the last safe town waits\n");
    printf("      *     /\\    .         beyond the mud road\n");
    printf("         /~~~~\\      *\n");
    printf("        /      \\  .\n");
    printf("       /___  ___\\      *\n");
    printf("           ||\n");
    printf("          _||_\n");
    printf("\n");
}

void asciiTownGate(void)
{
    printf("\n");
    printf("          _______________________________          \n");
    printf("         /                               \\         \n");
    printf("        |     E L D E R B R O O K         |        \n");
    printf("        |   the last safe town            |        \n");
    printf("         \\_______                 _______/         \n");
    printf("                 |               |                 \n");
    printf("                 |   [ GATE ]    |                 \n");
    printf("                 |               |                 \n");
    printf("        ~~~~~~~~~/               \\~~~~~~~~~        \n");
    printf("         muddy road · lantern light · wind         \n");
    printf("\n");
}

void asciiWorldMapHeader(void)
{
    asciiBanner("WORLD MAP");
    printf("  Roads open as you clear each location. Guild houses mark the map.\n");
}

void asciiCombatHeader(const char *enemyName, int enemyLevel)
{
    printf("\n");
    printf("     /\\                                                    /\\\n");
    printf("    //\\\\   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   //\\\\\n");
    printf("   //  \\\\                   C O M B A T                  //  \\\\\n");
    printf("  //____\\\\  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //____\\\\\n");
    printf("     ||                                                      ||\n");
    printf("     ||   vs  %-32s  Lv %-2d  ||\n", enemyName, enemyLevel);
    printf("     ||                                                      ||\n");
    printf("  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void asciiVictory(void)
{
    printf("\n");
    printf("   *  .  *        V I C T O R Y        *  .  *\n");
    printf("      \\|/                                \\|/\n");
    printf("     --*--                              --*--\n");
    printf("      /|\\                                /|\\\n");
    printf("\n");
}

void asciiDefeat(void)
{
    printf("\n");
    printf("   .  .  .         D E F E A T          .  .  .\n");
    printf("      X                                    X\n");
    printf("     / \\                                  / \\\n");
    printf("\n");
}

void asciiLocationArt(int level)
{
    /* Compact location vignettes — one short block each. */
    switch (level)
    {
    case 1:  /* Whispering Forest */
        printf("      .  *  .          /\\\n");
        printf("    *   /\\   *       /~~\\\n");
        printf("       /~~\\    .    /    \\\n");
        printf("   .  /    \\ *     /~~~~~~\\\n");
        break;
    case 4:  /* Castle of Ash */
        printf("         |^^^|  |^^^|\n");
        printf("         |   |--|   |\n");
        printf("         | []    [] |\n");
        printf("         |__________|\n");
        break;
    case 8:  /* Dragon's Forge */
        printf("        (  ~~~~  )\n");
        printf("       (  fire   )\n");
        printf("        \\ ~~~~  /\n");
        printf("         ||||||\n");
        break;
    case 15: /* Final Citadel */
        printf("           /\\    /\\\n");
        printf("          /  \\__/  \\\n");
        printf("         /__________\\\n");
        printf("         |  CITADEL |\n");
        break;
    default:
        printf("      .     *        .\n");
        printf("         /\\      *\n");
        printf("        /  \\   .\n");
        break;
    }
}

void asciiChessHeader(void)
{
    asciiBanner("CHESS DUEL");
    printf("  You are White (capitals). Moves: e2 e4   Type quit to leave.\n");
}

void asciiSnakeHeader(void)
{
    asciiBanner("SNAKE DUEL");
    printf("  @ head  o body  * fruit   w a s d to move   q to give up\n");
}

void asciiDiceHeader(void)
{
    asciiBanner("DICE DUEL");
    printf("  Three dice each. Re-roll any once. Higher total wins the round.\n");
}

void asciiBlackjackHeader(void)
{
    asciiBanner("BLACKJACK DUEL");
    printf("  Get closer to 21 without going over. First to 2 hands wins.\n");
}

void asciiArcheryHeader(void)
{
    asciiBanner("ARCHERY");
    printf("  Aim column then row (e.g. 3 2). Wind pushes sideways — aim against it.\n");
}

void asciiFishingHeader(void)
{
    asciiBanner("FISHING");
    printf("  Cast, wait, reel. Ease off when tension climbs. Don't snap the line.\n");
}

void asciiMemoryHeader(void)
{
    asciiBanner("MEMORY TRIAL");
    printf("  Watch the pattern, then repeat it from memory.\n");
}

void asciiReflexHeader(void)
{
    asciiBanner("REFLEX CHALLENGE");
    printf("  Match the shouted directions before time runs out.\n");
}

void asciiLockpickHeader(void)
{
    asciiBanner("LOCKPICKING");
    printf("  Find the pin code using the clues from each attempt.\n");
}

void asciiBar(const char *label, int cur, int max, int width)
{
    if (width < 4) width = 4;
    if (max < 1) max = 1;
    if (cur < 0) cur = 0;
    if (cur > max) cur = max;

    int filled = (cur * width) / max;
    printf("%s [", label);
    for (int i = 0; i < width; i++)
        putchar(i < filled ? '#' : '.');
    printf("] %d/%d\n", cur, max);
}

void asciiMenuFrame(const char *title)
{
    asciiBanner(title);
}
