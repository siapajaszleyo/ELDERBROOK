#ifndef SNAKE_H
#define SNAKE_H

/* Snake minigame against a miniboss.

   You start 2 metres long. Each fruit adds 1 metre. Reach 15 metres to
   win. Part way through, the miniboss cuts you down to 1 metre!
   Hitting a wall or yourself loses.

   Returns MINI_WIN, MINI_LOSE or MINI_QUIT (see game.h). */
int playSnake(int dungeonLevel, const char *bossName);

#endif
