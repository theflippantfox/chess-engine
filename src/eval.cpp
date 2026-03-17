#include "eval.h"

// Piece base values in centipawns
static const int pieceValue[] = {0, 100,  320,  330,  500,  900,  20000, 0,
                                 0, -100, -320, -330, -500, -900, -20000};

// Tables are from White's perspective, a1=index 0, h8=index 63
// We'll mirror them for Black

// Pawns: advance and control center
static const int pawnTable[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0,
};

// Knights: bad on edges, great in center
static const int knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
};

// Bishops: diagonals and open positions
static const int bishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
};

// Rooks: open files and 7th rank
static const int rookTable[64] = {
    0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0,
};

// Queen: flexible, avoid early development
static const int queenTable[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20,
};

// King middlegame: stay castled and safe
static const int kingTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20,
};

// Mirror a square for Black (flip rank)
static inline int mirror(Square sq) { return (7 - sq / 8) * 8 + (sq % 8); }

static int pieceSquareBonus(Piece p, Square sq) {
  switch (p) {
  case W_PAWN:
    return pawnTable[sq];
  case B_PAWN:
    return -pawnTable[mirror(sq)];
  case W_KNIGHT:
    return knightTable[sq];
  case B_KNIGHT:
    return -knightTable[mirror(sq)];
  case W_BISHOP:
    return bishopTable[sq];
  case B_BISHOP:
    return -bishopTable[mirror(sq)];
  case W_ROOK:
    return rookTable[sq];
  case B_ROOK:
    return -rookTable[mirror(sq)];
  case W_QUEEN:
    return queenTable[sq];
  case B_QUEEN:
    return -queenTable[mirror(sq)];
  case W_KING:
    return kingTable[sq];
  case B_KING:
    return -kingTable[mirror(sq)];
  default:
    return 0;
  }
}

int evaluate(const Board &b) {
  int score = 0;
  for (Square sq = 0; sq < 64; ++sq) {
    Piece p = b.at(sq);
    if (isEmpty(p))
      continue;
    score += pieceValue[p];
    score += pieceSquareBonus(p, sq);
  }
  return score;
}
