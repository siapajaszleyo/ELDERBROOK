#ifndef UI_H
#define UI_H

/* Heart and gem symbols. If they show up as garbage on your computer,
   compile with  -DASCII_ONLY  to use plain characters instead. */
#ifdef ASCII_ONLY
#define HEART  "#"
#define GEM    "*"
#else
#define HEART  "\xE2\x99\xA5"
#define GEM    "\xE2\x99\xA6"
#endif

void showHealthBar(void);
void showManaMeter(void);
void showKeys(void);
void showStats(void);
void uiRule(void);
void uiTitle(const char *title);
void uiPanelLine(const char *text);

#endif
