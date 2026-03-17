#pragma once
#include <cstdint>

enum Piece : uint8_t {
  EMPTY = 0,
  W_PAWN = 1,
  W_KNIGHT,
  W_BISHOP,
  W_ROOK,
  W_QUEEN,
  W_KING,
  B_PAWN = 9,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_QUEEN,
  B_KING,
};

enum Color {
  WHITE = 0,
  BLACK = 1,
};

using Square = int;
constexpr Square NO_SQ = -1;

inline bool isWhitePiece(Piece p) { return p >= W_PAWN && p <= W_KING; }
inline bool isBlackPiece(Piece p) { return p >= B_PAWN && p <= B_KING; }
inline Color pieceColor(Piece p) { return isBlackPiece(p) ? BLACK : WHITE; }
inline bool isEmpty(Piece p) { return p == EMPTY; }

inline bool isFriendly(Piece p, Color us) {
  if (isEmpty(p))
    return false;
  return us == WHITE ? (p >= W_PAWN && p <= W_KING)
                     : (p >= B_PAWN && p <= B_KING);
}

struct Move {
  Square from, to;
  Piece promotion;

  Move() : from(NO_SQ), to(NO_SQ), promotion(EMPTY) {}
  Move(Square f, Square t, Piece promo = EMPTY)
      : from(f), to(t), promotion(promo) {}

  bool operator==(const Move &o) const {
    return from == o.from && to == o.to && promotion == o.promotion;
  }
};
