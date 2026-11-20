#ifndef DICE_H
#define DICE_H

#define DICE_QUIT   -1
#define DICE_LOSE    0
#define DICE_WIN     1
#define DICE_TIE     2

/* One round of three-dice poker-style rolling: both sides roll three
   dice, you may re-roll any of yours once, the higher total wins.
   Returns DICE_WIN, DICE_LOSE, DICE_TIE or DICE_QUIT. */
int diceRound(const char *opponent, int dungeonLevel);

/* A miniboss duel: first to win 3 rounds wins.
   Returns MINI_WIN, MINI_LOSE or MINI_QUIT (see game.h). */
int playDice(int dungeonLevel, const char *bossName);

#endif
