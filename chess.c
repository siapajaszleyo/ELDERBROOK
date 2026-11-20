/* =========================================================
   chess.c  -  text chess with a computer opponent

   Your original rules engine, plus:
     - applyMove()   : moves a piece with NO printing/input,
                       so the bot can "try" moves quietly
     - generateMoves(): lists every legal move for one side
     - evaluate()    : scores a position (who is winning?)
     - negamax()     : the bot's "thinking" (minimax with
                       alpha-beta pruning)
     - playChess()   : one whole game, called from the RPG

   Everything is 'static' = private to this file, so it can't
   clash with names in your RPG. Only playChess() is public.
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "chess.h"
#include "ascii.h"

#define BOARD_SIZE 8
#define MAX_MOVES 256
#define MATE_SCORE 100000
#define INFINITY_SCORE 1000000
#define MAX_HALF_MOVES 200      /* after this many moves: draw */

static char board[BOARD_SIZE][BOARD_SIZE];

static bool whiteTurn = true;

static bool whiteKingMoved = false;
static bool blackKingMoved = false;

static bool whiteRookAMoved = false;
static bool whiteRookHMoved = false;
static bool blackRookAMoved = false;
static bool blackRookHMoved = false;

static int enPassantRow = -1;
static int enPassantCol = -1;

/* Who the human is playing against (shown in messages) */
static const char *opponentName = "The boss";

void setChessOpponent(const char *name)
{
    opponentName = (name != NULL) ? name : "The boss";
}


/* A move is 5 related values, so we bundle them in a struct */
typedef struct
{
    int fromRow, fromCol;
    int toRow, toCol;
    char promo;             /* 'Q' if this move promotes a pawn, else 0 */
} Move;

/* A snapshot of the whole game, so the bot can undo moves */
typedef struct
{
    char board[BOARD_SIZE][BOARD_SIZE];
    bool whiteTurn;
    bool whiteKingMoved, blackKingMoved;
    bool whiteRookAMoved, whiteRookHMoved;
    bool blackRookAMoved, blackRookHMoved;
    int enPassantRow, enPassantCol;
} GameState;


/* =========================================================
   PIECE HELPERS
   ========================================================= */

static bool isWhitePiece(char piece)
{
    return piece >= 'A' && piece <= 'Z';
}

static bool isBlackPiece(char piece)
{
    return piece >= 'a' && piece <= 'z';
}

static bool isOpponentPiece(char piece, bool white)
{
    if (piece == '.')
        return false;

    if (white)
        return isBlackPiece(piece);

    return isWhitePiece(piece);
}

static bool isOwnPiece(char piece, bool white)
{
    if (piece == '.')
        return false;

    if (white)
        return isWhitePiece(piece);

    return isBlackPiece(piece);
}


/* =========================================================
   SET UP AND DRAW THE BOARD
   ========================================================= */

static void initializeBoard(void)
{
    for (int row = 0; row < BOARD_SIZE; row++)
        for (int col = 0; col < BOARD_SIZE; col++)
            board[row][col] = '.';

    /* White pieces */
    board[0][0] = 'R';
    board[0][1] = 'N';
    board[0][2] = 'B';
    board[0][3] = 'Q';
    board[0][4] = 'K';
    board[0][5] = 'B';
    board[0][6] = 'N';
    board[0][7] = 'R';

    for (int col = 0; col < BOARD_SIZE; col++)
        board[1][col] = 'P';

    /* Black pieces */
    board[7][0] = 'r';
    board[7][1] = 'n';
    board[7][2] = 'b';
    board[7][3] = 'q';
    board[7][4] = 'k';
    board[7][5] = 'b';
    board[7][6] = 'n';
    board[7][7] = 'r';

    for (int col = 0; col < BOARD_SIZE; col++)
        board[6][col] = 'p';

    whiteTurn = true;

    whiteKingMoved = false;
    blackKingMoved = false;

    whiteRookAMoved = false;
    whiteRookHMoved = false;
    blackRookAMoved = false;
    blackRookHMoved = false;

    enPassantRow = -1;
    enPassantCol = -1;
}

static void printBoard(void)
{
    printf("\n");
    printf("    a b c d e f g h\n");
    printf("  +-----------------+\n");

    for (int row = BOARD_SIZE - 1; row >= 0; row--)
    {
        printf("%d | ", row + 1);

        for (int col = 0; col < BOARD_SIZE; col++)
            printf("%c ", board[row][col]);

        printf("| %d\n", row + 1);
    }

    printf("  +-----------------+\n");
    printf("    a b c d e f g h\n");
}

static bool parseSquare(const char *text, int *row, int *col)
{
    if (strlen(text) != 2)
        return false;

    char file = (char)tolower((unsigned char)text[0]);
    char rank = text[1];

    if (file < 'a' || file > 'h')
        return false;

    if (rank < '1' || rank > '8')
        return false;

    *col = file - 'a';
    *row = rank - '1';

    return true;
}


/* =========================================================
   ATTACK / CHECK DETECTION
   ========================================================= */

static bool isPathClear(int fromRow, int fromCol,
                        int toRow, int toCol)
{
    int rowStep = 0;
    int colStep = 0;

    if (toRow > fromRow)
        rowStep = 1;
    else if (toRow < fromRow)
        rowStep = -1;

    if (toCol > fromCol)
        colStep = 1;
    else if (toCol < fromCol)
        colStep = -1;

    int row = fromRow + rowStep;
    int col = fromCol + colStep;

    while (row != toRow || col != toCol)
    {
        if (board[row][col] != '.')
            return false;

        row += rowStep;
        col += colStep;
    }

    return true;
}

static bool findKing(bool white, int *kingRow, int *kingCol)
{
    char king = white ? 'K' : 'k';

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board[row][col] == king)
            {
                *kingRow = row;
                *kingCol = col;
                return true;
            }
        }
    }

    return false;
}

static bool squareIsAttacked(int targetRow, int targetCol, bool byWhite)
{
    /* Pawns */
    if (byWhite)
    {
        int pawnRow = targetRow - 1;

        if (pawnRow >= 0)
        {
            if (targetCol - 1 >= 0 &&
                board[pawnRow][targetCol - 1] == 'P')
                return true;

            if (targetCol + 1 < BOARD_SIZE &&
                board[pawnRow][targetCol + 1] == 'P')
                return true;
        }
    }
    else
    {
        int pawnRow = targetRow + 1;

        if (pawnRow < BOARD_SIZE)
        {
            if (targetCol - 1 >= 0 &&
                board[pawnRow][targetCol - 1] == 'p')
                return true;

            if (targetCol + 1 < BOARD_SIZE &&
                board[pawnRow][targetCol + 1] == 'p')
                return true;
        }
    }

    /* Knights */
    int knightMoves[8][2] =
    {
        { 2, 1 }, { 2, -1 }, {-2, 1 }, {-2, -1 },
        { 1, 2 }, { 1, -2 }, {-1, 2 }, {-1, -2 }
    };

    char knight = byWhite ? 'N' : 'n';

    for (int i = 0; i < 8; i++)
    {
        int row = targetRow + knightMoves[i][0];
        int col = targetCol + knightMoves[i][1];

        if (row >= 0 && row < 8 &&
            col >= 0 && col < 8 &&
            board[row][col] == knight)
        {
            return true;
        }
    }

    /* King */
    char king = byWhite ? 'K' : 'k';

    for (int row = targetRow - 1; row <= targetRow + 1; row++)
    {
        for (int col = targetCol - 1; col <= targetCol + 1; col++)
        {
            if (row >= 0 && row < 8 &&
                col >= 0 && col < 8 &&
                board[row][col] == king)
            {
                return true;
            }
        }
    }

    /* Rooks and queens (straight lines) */
    char rook = byWhite ? 'R' : 'r';
    char queen = byWhite ? 'Q' : 'q';

    int straightDirections[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    for (int d = 0; d < 4; d++)
    {
        int row = targetRow + straightDirections[d][0];
        int col = targetCol + straightDirections[d][1];

        while (row >= 0 && row < 8 && col >= 0 && col < 8)
        {
            char piece = board[row][col];

            if (piece != '.')
            {
                if (piece == rook || piece == queen)
                    return true;

                break;
            }

            row += straightDirections[d][0];
            col += straightDirections[d][1];
        }
    }

    /* Bishops and queens (diagonals) */
    char bishop = byWhite ? 'B' : 'b';

    int diagonalDirections[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

    for (int d = 0; d < 4; d++)
    {
        int row = targetRow + diagonalDirections[d][0];
        int col = targetCol + diagonalDirections[d][1];

        while (row >= 0 && row < 8 && col >= 0 && col < 8)
        {
            char piece = board[row][col];

            if (piece != '.')
            {
                if (piece == bishop || piece == queen)
                    return true;

                break;
            }

            row += diagonalDirections[d][0];
            col += diagonalDirections[d][1];
        }
    }

    return false;
}

static bool isCheck(bool white)
{
    int kingRow;
    int kingCol;

    if (!findKing(white, &kingRow, &kingCol))
        return true;

    return squareIsAttacked(kingRow, kingCol, !white);
}


/* =========================================================
   MOVE RULES
   ========================================================= */

static bool basicMoveValid(int fromRow, int fromCol,
                           int toRow, int toCol,
                           bool white)
{
    char piece = board[fromRow][fromCol];
    char destination = board[toRow][toCol];

    int rowDiff = abs(toRow - fromRow);
    int colDiff = abs(toCol - fromCol);

    /* Cannot capture own piece */
    if (isOwnPiece(destination, white))
        return false;

    /* PAWN */
    if (piece == 'P' || piece == 'p')
    {
        int direction = white ? 1 : -1;
        int startingRow = white ? 1 : 6;

        /* Move forward one */
        if (toCol == fromCol &&
            toRow == fromRow + direction &&
            destination == '.')
            return true;

        /* Move forward two */
        if (fromRow == startingRow &&
            toCol == fromCol &&
            toRow == fromRow + 2 * direction &&
            destination == '.' &&
            board[fromRow + direction][fromCol] == '.')
            return true;

        /* Diagonal capture */
        if (colDiff == 1 &&
            toRow == fromRow + direction &&
            isOpponentPiece(destination, white))
            return true;

        /* En passant */
        if (colDiff == 1 &&
            toRow == fromRow + direction &&
            destination == '.' &&
            toRow == enPassantRow &&
            toCol == enPassantCol)
            return true;

        return false;
    }

    /* KNIGHT */
    if (piece == 'N' || piece == 'n')
        return (rowDiff == 2 && colDiff == 1) ||
               (rowDiff == 1 && colDiff == 2);

    /* BISHOP */
    if (piece == 'B' || piece == 'b')
        return rowDiff == colDiff &&
               rowDiff > 0 &&
               isPathClear(fromRow, fromCol, toRow, toCol);

    /* ROOK */
    if (piece == 'R' || piece == 'r')
        return (fromRow == toRow || fromCol == toCol) &&
               !(fromRow == toRow && fromCol == toCol) &&
               isPathClear(fromRow, fromCol, toRow, toCol);

    /* QUEEN */
    if (piece == 'Q' || piece == 'q')
    {
        bool straight = (fromRow == toRow || fromCol == toCol);
        bool diagonal = (rowDiff == colDiff);

        return (straight || diagonal) &&
               !(fromRow == toRow && fromCol == toCol) &&
               isPathClear(fromRow, fromCol, toRow, toCol);
    }

    /* KING */
    if (piece == 'K' || piece == 'k')
        return rowDiff <= 1 &&
               colDiff <= 1 &&
               !(rowDiff == 0 && colDiff == 0);

    return false;
}

static void copyBoard(char destination[8][8], char source[8][8])
{
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            destination[row][col] = source[row][col];
}

/* Try the move, see if our own king is attacked, then undo it */
static bool moveLeavesKingSafe(int fromRow, int fromCol,
                               int toRow, int toCol,
                               bool white)
{
    char originalBoard[8][8];

    copyBoard(originalBoard, board);

    char piece = board[fromRow][fromCol];

    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = '.';

    /* En passant capture removes a pawn from a different square */
    if ((piece == 'P' || piece == 'p') &&
        toCol != fromCol &&
        originalBoard[toRow][toCol] == '.' &&
        toRow == enPassantRow &&
        toCol == enPassantCol)
    {
        int capturedPawnRow = white ? toRow - 1 : toRow + 1;
        board[capturedPawnRow][toCol] = '.';
    }

    bool safe = !isCheck(white);

    copyBoard(board, originalBoard);

    return safe;
}

static bool canCastle(int fromRow, int fromCol,
                      int toRow, int toCol,
                      bool white)
{
    if (fromCol != 4)
        return false;

    if (toCol != 2 && toCol != 6)
        return false;

    int row = white ? 0 : 7;

    if (fromRow != row || toRow != row)
        return false;

    if (isCheck(white))
        return false;

    /* King side */
    if (toCol == 6)
    {
        if (white)
        {
            if (whiteKingMoved || whiteRookHMoved || board[0][7] != 'R')
                return false;
        }
        else
        {
            if (blackKingMoved || blackRookHMoved || board[7][7] != 'r')
                return false;
        }

        if (board[row][5] != '.' || board[row][6] != '.')
            return false;

        if (squareIsAttacked(row, 5, !white) ||
            squareIsAttacked(row, 6, !white))
            return false;

        return true;
    }

    /* Queen side */
    if (white)
    {
        if (whiteKingMoved || whiteRookAMoved || board[0][0] != 'R')
            return false;
    }
    else
    {
        if (blackKingMoved || blackRookAMoved || board[7][0] != 'r')
            return false;
    }

    if (board[row][1] != '.' || board[row][2] != '.' || board[row][3] != '.')
        return false;

    if (squareIsAttacked(row, 3, !white) ||
        squareIsAttacked(row, 2, !white))
        return false;

    return true;
}

static bool isValidMove(int fromRow, int fromCol,
                        int toRow, int toCol,
                        bool white)
{
    if (fromRow < 0 || fromRow >= 8 ||
        fromCol < 0 || fromCol >= 8 ||
        toRow < 0 || toRow >= 8 ||
        toCol < 0 || toCol >= 8)
        return false;

    char piece = board[fromRow][fromCol];

    if (piece == '.')
        return false;

    if (!isOwnPiece(piece, white))
        return false;

    /* Castling */
    if ((piece == 'K' || piece == 'k') && abs(toCol - fromCol) == 2)
        return canCastle(fromRow, fromCol, toRow, toCol, white);

    /* Normal movement */
    if (!basicMoveValid(fromRow, fromCol, toRow, toCol, white))
        return false;

    /* Cannot capture the enemy king directly */
    if (board[toRow][toCol] == 'K' || board[toRow][toCol] == 'k')
        return false;

    /* Own king must not be left in check */
    if (!moveLeavesKingSafe(fromRow, fromCol, toRow, toCol, white))
        return false;

    return true;
}


/* =========================================================
   APPLYING MOVES  (no printing, no input: safe for the bot)
   ========================================================= */

static void applyMove(int fromRow, int fromCol,
                      int toRow, int toCol,
                      char promo)
{
    char piece = board[fromRow][fromCol];
    bool white = isWhitePiece(piece);

    /* En passant capture */
    if ((piece == 'P' || piece == 'p') &&
        toCol != fromCol &&
        board[toRow][toCol] == '.' &&
        toRow == enPassantRow &&
        toCol == enPassantCol)
    {
        int capturedPawnRow = white ? toRow - 1 : toRow + 1;
        board[capturedPawnRow][toCol] = '.';
    }

    /* Move the piece */
    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = '.';

    /* Castling also moves the rook */
    if ((piece == 'K' || piece == 'k') && abs(toCol - fromCol) == 2)
    {
        if (toCol == 6)
        {
            board[toRow][5] = board[toRow][7];
            board[toRow][7] = '.';
        }
        else
        {
            board[toRow][3] = board[toRow][0];
            board[toRow][0] = '.';
        }
    }

    /* Castling rights: king or rook moved... */
    if (piece == 'K')
        whiteKingMoved = true;

    if (piece == 'k')
        blackKingMoved = true;

    if (fromRow == 0 && fromCol == 0) whiteRookAMoved = true;
    if (fromRow == 0 && fromCol == 7) whiteRookHMoved = true;
    if (fromRow == 7 && fromCol == 0) blackRookAMoved = true;
    if (fromRow == 7 && fromCol == 7) blackRookHMoved = true;

    /* ...or a rook was captured on its home square */
    if (toRow == 0 && toCol == 0) whiteRookAMoved = true;
    if (toRow == 0 && toCol == 7) whiteRookHMoved = true;
    if (toRow == 7 && toCol == 0) blackRookAMoved = true;
    if (toRow == 7 && toCol == 7) blackRookHMoved = true;

    /* En passant target for the next move */
    enPassantRow = -1;
    enPassantCol = -1;

    if (piece == 'P' && fromRow == 1 && toRow == 3)
    {
        enPassantRow = 2;
        enPassantCol = fromCol;
    }

    if (piece == 'p' && fromRow == 6 && toRow == 4)
    {
        enPassantRow = 5;
        enPassantCol = fromCol;
    }

    /* Promotion (defaults to a queen) */
    if (piece == 'P' && toRow == 7)
        board[toRow][toCol] = promo ? (char)toupper((unsigned char)promo) : 'Q';

    if (piece == 'p' && toRow == 0)
        board[toRow][toCol] = promo ? (char)tolower((unsigned char)promo) : 'q';

    whiteTurn = !whiteTurn;
}

/* Ask the human what to promote to, then make the move */
static void makeHumanMove(int fromRow, int fromCol, int toRow, int toCol)
{
    char piece = board[fromRow][fromCol];
    char promo = 0;

    if ((piece == 'P' && toRow == 7) || (piece == 'p' && toRow == 0))
    {
        char choice = 'Q';
        char line[64];

        printf("Promote pawn to (Q/R/B/N): ");
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) != NULL)
        {
            for (int i = 0; line[i] != '\0'; i++)
            {
                if (!isspace((unsigned char)line[i]))
                {
                    choice = (char)toupper((unsigned char)line[i]);
                    break;
                }
            }
        }

        if (choice != 'Q' && choice != 'R' &&
            choice != 'B' && choice != 'N')
            choice = 'Q';

        promo = choice;
    }

    applyMove(fromRow, fromCol, toRow, toCol, promo);
}


/* =========================================================
   SAVE / RESTORE THE WHOLE GAME (lets the bot undo moves)
   ========================================================= */

static void saveState(GameState *s)
{
    memcpy(s->board, board, sizeof(board));
    s->whiteTurn = whiteTurn;
    s->whiteKingMoved = whiteKingMoved;
    s->blackKingMoved = blackKingMoved;
    s->whiteRookAMoved = whiteRookAMoved;
    s->whiteRookHMoved = whiteRookHMoved;
    s->blackRookAMoved = blackRookAMoved;
    s->blackRookHMoved = blackRookHMoved;
    s->enPassantRow = enPassantRow;
    s->enPassantCol = enPassantCol;
}

static void loadState(const GameState *s)
{
    memcpy(board, s->board, sizeof(board));
    whiteTurn = s->whiteTurn;
    whiteKingMoved = s->whiteKingMoved;
    blackKingMoved = s->blackKingMoved;
    whiteRookAMoved = s->whiteRookAMoved;
    whiteRookHMoved = s->whiteRookHMoved;
    blackRookAMoved = s->blackRookAMoved;
    blackRookHMoved = s->blackRookHMoved;
    enPassantRow = s->enPassantRow;
    enPassantCol = s->enPassantCol;
}


/* =========================================================
   LEGAL MOVES AND GAME OVER
   ========================================================= */

/* Fill moves[] with every legal move for this side; return how many */
static int generateMoves(bool white, Move moves[])
{
    int count = 0;

    for (int fromRow = 0; fromRow < 8; fromRow++)
    {
        for (int fromCol = 0; fromCol < 8; fromCol++)
        {
            char piece = board[fromRow][fromCol];

            if (!isOwnPiece(piece, white))
                continue;

            for (int toRow = 0; toRow < 8; toRow++)
            {
                for (int toCol = 0; toCol < 8; toCol++)
                {
                    if (!isValidMove(fromRow, fromCol, toRow, toCol, white))
                        continue;

                    Move m = { fromRow, fromCol, toRow, toCol, 0 };

                    if ((piece == 'P' && toRow == 7) ||
                        (piece == 'p' && toRow == 0))
                        m.promo = 'Q';

                    moves[count] = m;
                    count++;
                }
            }
        }
    }

    return count;
}

static bool hasLegalMove(bool white)
{
    for (int fromRow = 0; fromRow < 8; fromRow++)
        for (int fromCol = 0; fromCol < 8; fromCol++)
        {
            if (!isOwnPiece(board[fromRow][fromCol], white))
                continue;

            for (int toRow = 0; toRow < 8; toRow++)
                for (int toCol = 0; toCol < 8; toCol++)
                    if (isValidMove(fromRow, fromCol, toRow, toCol, white))
                        return true;
        }

    return false;
}

/* 0 = game goes on, 1 = White wins, 2 = Black wins, 3 = stalemate */
static int gameStatus(void)
{
    bool white = whiteTurn;

    if (hasLegalMove(white))
        return 0;

    if (isCheck(white))
        return white ? 2 : 1;

    return 3;
}


/* =========================================================
   THE BOT
   ========================================================= */

static int pieceValue(char piece)
{
    switch (tolower((unsigned char)piece))
    {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
        default:  return 0;      /* kings can't be captured */
    }
}

/* Score the position from WHITE's point of view.
   Positive = White is ahead, negative = Black is ahead. */
static int evaluate(void)
{
    int score = 0;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            char piece = board[row][col];

            if (piece == '.')
                continue;

            int value = pieceValue(piece);
            char kind = (char)tolower((unsigned char)piece);

            /* 0 in the exact middle of the board, larger at the edges */
            int distance = abs(2 * row - 7) + abs(2 * col - 7);

            if (kind == 'n' || kind == 'b')
                value += (14 - distance) * 2;         /* like the centre */

            if (kind == 'p')
            {
                int advance = isWhitePiece(piece) ? row - 1 : 6 - row;
                value += advance * 6;                 /* pawns should advance */
            }

            score += isWhitePiece(piece) ? value : -value;
        }
    }

    return score;
}

/* Score for the side to move ("white" says who that is).
   Looks 'depth' moves ahead, assuming both sides play their best.
   alpha/beta let it skip branches that can't matter (pruning). */
static int negamax(int depth, int alpha, int beta, bool white, int ply)
{
    if (depth == 0)
        return white ? evaluate() : -evaluate();

    Move moves[MAX_MOVES];
    int count = generateMoves(white, moves);

    if (count == 0)
    {
        if (isCheck(white))
            return -(MATE_SCORE - ply);    /* checkmated: sooner = worse */

        return 0;                          /* stalemate */
    }

    GameState saved;
    saveState(&saved);

    int best = -INFINITY_SCORE;

    for (int i = 0; i < count; i++)
    {
        applyMove(moves[i].fromRow, moves[i].fromCol,
                  moves[i].toRow, moves[i].toCol,
                  moves[i].promo);

        int score = -negamax(depth - 1, -beta, -alpha, !white, ply + 1);

        loadState(&saved);

        if (score > best)
            best = score;

        if (best > alpha)
            alpha = best;

        if (alpha >= beta)
            break;
    }

    return best;
}

/* Pick the bot's move. Level 1 sometimes blunders on purpose. */
static Move chooseBotMove(bool white, int depth, int level)
{
    Move moves[MAX_MOVES];
    int count = generateMoves(white, moves);

    if (level == 1 && rand() % 4 == 0)
        return moves[rand() % count];

    GameState saved;
    saveState(&saved);

    int bestScore = -INFINITY_SCORE - 1;
    int bestIndexes[MAX_MOVES];
    int bestCount = 0;

    for (int i = 0; i < count; i++)
    {
        applyMove(moves[i].fromRow, moves[i].fromCol,
                  moves[i].toRow, moves[i].toCol,
                  moves[i].promo);

        int score = -negamax(depth - 1, -INFINITY_SCORE, INFINITY_SCORE,
                             !white, 1);

        loadState(&saved);

        if (score > bestScore)
        {
            bestScore = score;
            bestCount = 0;
        }

        if (score == bestScore)
        {
            bestIndexes[bestCount] = i;
            bestCount++;
        }
    }

    /* Several moves tied? Pick one at random so games vary */
    return moves[bestIndexes[rand() % bestCount]];
}


/* =========================================================
   PLAY ONE GAME  (you = White, computer = Black)
   ========================================================= */

int playChess(int level)
{
#ifdef RPG_TUI
    asciiChessHeader();
#endif
    if (level < 1) level = 1;
    if (level > 3) level = 3;

    initializeBoard();

    printf("\n=================================\n");
    printf("        CHESS DUEL\n");
    printf("=================================\n");
    printf("You are White (capital letters) and move first.\n");
    printf("Enter moves like:  e2 e4\n");
    printf("Type 'quit' to walk away from the board.\n");

    char fromText[10];
    char toText[10];
    char line[128];
    int halfMoves = 0;

    while (1)
    {
        printBoard();

        int status = gameStatus();

        if (status == 1)
        {
            printf("\n*** CHECKMATE! You win! ***\n");
            return CHESS_WIN;
        }

        if (status == 2)
        {
            printf("\n*** CHECKMATE! You lose. ***\n");
            return CHESS_LOSE;
        }

        if (status == 3)
        {
            printf("\n*** STALEMATE - it's a draw. ***\n");
            return CHESS_DRAW;
        }

        if (halfMoves >= MAX_HALF_MOVES)
        {
            printf("\n*** The game drags on too long - it's a draw. ***\n");
            return CHESS_DRAW;
        }

        if (isCheck(whiteTurn))
            printf("\n*** CHECK! ***\n");

        if (whiteTurn)
        {
            printf("\nYour move: ");
            fflush(stdout);

            if (fgets(line, sizeof line, stdin) == NULL)
                return CHESS_QUIT;

            fromText[0] = '\0';
            toText[0]   = '\0';

            int words = sscanf(line, "%9s %9s", fromText, toText);

            if (words < 1)
                continue;                    /* blank line: ask again */

            if (strcmp(fromText, "quit") == 0)
                return CHESS_QUIT;

            if (words < 2)
            {
                printf("Invalid input! Use the format: e2 e4\n");
                continue;
            }

            int fromRow, fromCol, toRow, toCol;

            if (!parseSquare(fromText, &fromRow, &fromCol) ||
                !parseSquare(toText, &toRow, &toCol))
            {
                printf("Invalid input! Use the format: e2 e4\n");
                continue;
            }

            if (!isValidMove(fromRow, fromCol, toRow, toCol, true))
            {
                printf("Invalid move!\n");
                continue;
            }

            makeHumanMove(fromRow, fromCol, toRow, toCol);
        }
        else
        {
            printf("\n%s is thinking...\n", opponentName);

            Move m = chooseBotMove(false, level, level);

            printf("%s plays: %c%c %c%c\n",
                   opponentName,
                   'a' + m.fromCol, '1' + m.fromRow,
                   'a' + m.toCol,   '1' + m.toRow);

            applyMove(m.fromRow, m.fromCol, m.toRow, m.toCol, m.promo);
        }

        halfMoves++;
    }
}


/* =========================================================
   Play chess on its own:
       gcc -DCHESS_STANDALONE chess.c -o chess
   ========================================================= */

#ifdef CHESS_STANDALONE
int main(void)
{
    int level;

    srand((unsigned)time(NULL));

    printf("Difficulty (1 = easy, 2 = medium, 3 = hard): ");

    if (scanf("%d", &level) != 1)
        level = 2;

    playChess(level);

    printf("\nThanks for playing!\n");

    return 0;
}
#endif
