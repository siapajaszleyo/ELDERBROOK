#ifndef ASCII_H
#define ASCII_H

/* =========================================================
   ASCII.H  —  Rich terminal art assets for the TUI layer
   ---------------------------------------------------------
   Pure data + drawing helpers. Does not change game logic.
   Used by tui.c and (optionally) ui.c for prettier screens.
   All art is 7-bit safe where possible; UTF-8 hearts/gems
   still come from ui.h when available.
   ========================================================= */

/* Clear the visible screen (ANSI) without requiring ncurses. */
void asciiClear(void);

/* Draw a horizontal rule of width `w` (clamped 8..78). */
void asciiRule(int w);

/* Double-line box title banner. */
void asciiBanner(const char *title);

/* Single-line box around a short status line. */
void asciiStatusBox(const char *line);

/* Large title-screen art + game name. */
void asciiTitleScreen(void);

/* Town gate / Elderbrook arrival. */
void asciiTownGate(void);

/* World-map header. */
void asciiWorldMapHeader(void);

/* Combat header art (crossed swords / shield). */
void asciiCombatHeader(const char *enemyName, int enemyLevel);

/* Victory / defeat banners. */
void asciiVictory(void);
void asciiDefeat(void);

/* Location-flavoured small vignette by dungeon level 1..15. */
void asciiLocationArt(int level);

/* Minigame headers. */
void asciiChessHeader(void);
void asciiSnakeHeader(void);
void asciiDiceHeader(void);
void asciiBlackjackHeader(void);
void asciiArcheryHeader(void);
void asciiFishingHeader(void);
void asciiMemoryHeader(void);
void asciiReflexHeader(void);
void asciiLockpickHeader(void);

/* Simple filled progress bar: label [####....] cur/max */
void asciiBar(const char *label, int cur, int max, int width);

/* Framed menu option list helper (prints numbers). */
void asciiMenuFrame(const char *title);

#endif
