#ifndef TUI_H
#define TUI_H

/* =========================================================
   TUI.H  —  Terminal UI shell for foot / modern terminals
   ---------------------------------------------------------
   Optional display layer. Does NOT replace game logic.
   - Detects terminal capabilities (UTF-8, colour, size)
   - Provides framed screens, status strip, pause helpers
   - Entry point for the rpg-tui binary wraps main flow
     with clearer presentation while calling existing code.
   ========================================================= */

/* Initialise TUI: UTF-8 locale hint, optional colour, clear. */
void tuiInit(void);

/* Tear down (restore terminal if needed). */
void tuiShutdown(void);

/* Full-screen clear + home cursor (ANSI). */
void tuiClear(void);

/* Draw a persistent-looking status strip at the top. */
void tuiStatusStrip(const char *name, int level, int hp, int maxHp,
                    int mp, int maxMp, int gold, int cleared, int total);

/* Pause with "Press Enter..." (uses existing input helpers). */
void tuiPressEnter(void);

/* Pretty section divider used between major screens. */
void tuiSection(const char *title);

/* Coloured text helpers (no-op if colour unavailable). */
void tuiColor(const char *ansiCode);   /* e.g. "\033[1;32m" */
void tuiReset(void);

/* Convenience colours */
#define TUI_GREEN   "\033[1;32m"
#define TUI_RED     "\033[1;31m"
#define TUI_YELLOW  "\033[1;33m"
#define TUI_CYAN    "\033[1;36m"
#define TUI_BOLD    "\033[1m"
#define TUI_DIM     "\033[2m"

/* 1 if we believe the terminal supports colour (TERM not dumb). */
int tuiHasColor(void);

#endif
