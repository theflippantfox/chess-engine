#include "eval.h"

static const int pieceValue[] = {
    0,      // EMPTY
    100,    // W_PAWN
    320,    // W_KNIGHT
    330,    // W_BISHOP
    500,    // W_ROOK
    900,    // W_QUEEN
    20000,  // W_KING
    0,      // padding (index 7)
    0,      // padding (index 8)
    -100,   // B_PAWN
    -320,   // B_KNIGHT
    -330,   // B_BISHOP
    -500,   // B_ROOK
    -900,   // B_QUEEN
    -20000, // B_KING
};

int evaluate(const Board &b) {
  int score = 0;
  for (Square sq = 0; sq < 64; ++sq)
    score += pieceValue[b.at(sq)];
  return score;
}
