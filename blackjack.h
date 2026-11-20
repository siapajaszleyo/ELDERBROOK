#ifndef BLACKJACK_H
#define BLACKJACK_H

#define BJ_QUIT       -1
#define BJ_LOSE        0
#define BJ_WIN         1
#define BJ_PUSH        2
#define BJ_BLACKJACK   3

/* One hand against the dealer for a bet. *net is set to the gold you win
   (positive) or lose (negative). Returns one of the BJ_ codes. */
int blackjackHand(int bet, int *net);

/* A miniboss duel with no gold at stake: first to win 2 hands wins.
   Returns MINI_WIN, MINI_LOSE or MINI_QUIT. */
int playBlackjackDuel(const char *bossName);

#endif
