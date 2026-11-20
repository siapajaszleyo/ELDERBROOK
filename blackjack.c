#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "blackjack.h"
#include "input.h"
#include "game.h"
#include "adventure.h"
#include "class.h"
#include "ascii.h"

typedef struct
{
    int rank;       /* 1 = ace, 2..10, 11 = J, 12 = Q, 13 = K */
    int suit;       /* 0..3                                   */
} Card;

static Card deck[52];
static int  deckPos;

static void shuffleDeck(void)
{
    int n = 0;

    for (int suit = 0; suit < 4; suit++)
        for (int rank = 1; rank <= 13; rank++)
        {
            deck[n].rank = rank;
            deck[n].suit = suit;
            n++;
        }

    for (int i = 51; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Card temp = deck[i];

        deck[i] = deck[j];
        deck[j] = temp;
    }

    deckPos = 0;
}

static Card draw(void)
{
    if (deckPos >= 52)
        shuffleDeck();

    return deck[deckPos++];
}

/* Aces count as 11 unless that would bust the hand. */
static int handValue(const Card *hand, int n)
{
    int total = 0;
    int aces = 0;

    for (int i = 0; i < n; i++)
    {
        int rank = hand[i].rank;

        if (rank == 1)
        {
            aces++;
            total += 11;
        }
        else if (rank >= 10)
            total += 10;
        else
            total += rank;
    }

    while (total > 21 && aces > 0)
    {
        total -= 10;
        aces--;
    }

    return total;
}

static void printCard(Card c)
{
    static const char *ranks[14] = { "?", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
    static const char suits[4] = { 'H', 'D', 'C', 'S' };

    printf("[%s%c] ", ranks[c.rank], suits[c.suit]);
}

static void printHand(const char *who, const Card *hand, int n, int hideSecond)
{
    printf("%-8s", who);

    for (int i = 0; i < n; i++)
    {
        if (hideSecond && i == 1)
            printf("[??] ");
        else
            printCard(hand[i]);
    }

    if (!hideSecond)
        printf(" = %d", handValue(hand, n));

    printf("\n");
}

int blackjackHand(int bet, int *net)
{
    Card player[12], dealer[12];
    int np = 0, nd = 0;

    shuffleDeck();

    player[np++] = draw();
    dealer[nd++] = draw();
    player[np++] = draw();
    dealer[nd++] = draw();

    *net = 0;

    printf("\n");
    printHand("Dealer:", dealer, nd, 1);
    printHand("You:", player, np, 0);

    int playerBJ = handValue(player, np) == 21;
    int dealerBJ = handValue(dealer, nd) == 21;

    if (playerBJ || dealerBJ)
    {
        printHand("Dealer:", dealer, nd, 0);

        if (playerBJ && dealerBJ)
        {
            printf("Both have blackjack. Push.\n");
            return BJ_PUSH;
        }
        if (playerBJ)
        {
            *net = bet * 3 / 2;
            printf("BLACKJACK! You win %d gold (3 to 2).\n", *net);
            return BJ_BLACKJACK;
        }

        *net = -bet;
        printf("Dealer has blackjack. You lose %d gold.\n", bet);
        return BJ_LOSE;
    }

    /* Your turn */
    while (1)
    {
        printf("(h)it, (s)tand or (q)uit: ");

        char line[16];
        readLine(line, sizeof line);

        char choice = (char)tolower((unsigned char)line[0]);

        if (choice == 'q')
        {
            *net = -bet;
            printf("You fold. The dealer keeps your %d gold.\n", bet);
            return BJ_QUIT;
        }

        if (choice == 's')
            break;

        if (choice == 'h')
        {
            player[np++] = draw();
            printHand("You:", player, np, 0);

            if (handValue(player, np) > 21)
            {
                *net = -bet;
                printf("BUST! You lose %d gold.\n", bet);
                return BJ_LOSE;
            }

            if (handValue(player, np) == 21)
                break;                          /* nothing to gain by hitting */
        }
        else
            printf("Type h, s or q.\n");
    }

    /* Dealer's turn: hits until 17 or more */
    printHand("Dealer:", dealer, nd, 0);

    while (handValue(dealer, nd) < 17)
    {
        dealer[nd++] = draw();
        printf("Dealer draws... ");
        printHand("Dealer:", dealer, nd, 0);
    }

    int mine = handValue(player, np);
    int theirs = handValue(dealer, nd);

    /* Gambler class + King's Coin: Lady Luck can nudge a close hand. */
    int gambleBonus = classGamblingBonus(adv.playerClass) + relicGamblingBonus();
    if (gambleBonus > 0 && theirs <= 21 && mine <= 21 && mine < theirs)
    {
        if (rand() % 100 < gambleBonus)
        {
            printf("[Lady Luck intervenes] You draw one last card...\n");
            player[np++] = draw();
            printHand("You:", player, np, 0);
            mine = handValue(player, np);
            if (mine > 21)
                printf("...and it busts you. Fortune is fickle.\n");
        }
    }

    if (theirs > 21)
    {
        *net = bet;
        printf("Dealer BUSTS! You win %d gold.\n", bet);
        return BJ_WIN;
    }
    if (mine > theirs)
    {
        *net = bet;
        printf("You win %d gold! (%d to %d)\n", bet, mine, theirs);
        return BJ_WIN;
    }
    if (mine < theirs)
    {
        *net = -bet;
        printf("You lose %d gold. (%d to %d)\n", bet, mine, theirs);
        return BJ_LOSE;
    }

    printf("Push. Your bet is returned. (%d each)\n", mine);
    return BJ_PUSH;
}

int playBlackjackDuel(const char *bossName)
{
#ifdef RPG_TUI
    asciiBlackjackHeader();
#endif
    int myWins = 0;
    int theirWins = 0;

    printf("\n=================================\n");
    printf("        BLACKJACK DUEL\n");
    printf("=================================\n");
    printf("Beat %s's dealer at blackjack: get closer to 21 without going over.\n", bossName);
    printf("First to win 2 hands takes the duel. Pushes are replayed.\n");

    while (myWins < 2 && theirWins < 2)
    {
        int net;

        printf("\n--- Score: You %d - %d %s ---\n", myWins, theirWins, bossName);

        int result = blackjackHand(0, &net);

        if (result == BJ_QUIT)
            return MINI_QUIT;

        if (result == BJ_WIN || result == BJ_BLACKJACK)
        {
            myWins++;
            printf("You take the hand!\n");
        }
        else if (result == BJ_LOSE)
        {
            theirWins++;
            printf("%s takes the hand.\n", bossName);
        }
        else
            printf("Replay the hand.\n");
    }

    return myWins >= 2 ? MINI_WIN : MINI_LOSE;
}
