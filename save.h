#ifndef SAVE_H
#define SAVE_H

#include "player.h"
#include "game.h"

/* Files are created in the folder the game is run from. */
#define SAVE_FILE    "savegame.sav"
#define SAVE_BACKUP  "savegame.bak"     /* the save BEFORE the latest one */
#define SAVE_TEMP    "savegame.tmp"

/* The save format has a version number on its first line:
     RPGSAVE 1  the original ten-dungeon game
     RPGSAVE 2  the Expanded Edition
   Versions 1 through 6 remain loadable; omitted fields keep safe defaults.
   Current saves use version 7.

   The Adventure data (equipment, quests, ...) is saved together with the
   Player and Progress; it lives in the global "adv" (see adventure.h). */

/* Write everything to disk. The previous save is kept as savegame.bak.
   Returns 1 on success, 0 on failure. */
int saveGame(const Player *player, const Progress *progress);

/* Load a save. Returns 0 = nothing usable, 1 = loaded the main save,
   2 = the main save was damaged so the backup was loaded instead. */
int loadGame(Player *player, Progress *progress);

/* 1 if any save file exists */
int saveExists(void);

/* Delete the save files (used after the game is completed) */
void deleteSave(void);

#endif
