#ifndef CHESS_H
#define CHESS_H

/* Results returned by playChess() */
#define CHESS_LOSE 0
#define CHESS_WIN  1
#define CHESS_DRAW 2
#define CHESS_QUIT 3

/*
   Play one full game of chess in the terminal.
   You are White and type moves like:  e2 e4
   The computer plays Black.

   level: 1 = easy, 2 = medium, 3 = hard

   Returns CHESS_WIN, CHESS_LOSE, CHESS_DRAW or CHESS_QUIT.
   Call srand() once before using it (the RPG already does).
*/
int playChess(int level);

/* Optional: set the name shown for the computer player, e.g. "The Final Boss" */
void setChessOpponent(const char *name);

#endif
