#pragma once

#include "move.h"
#include <string>
#include <vector>

struct BoardState {
  Color sideToMove = WHITE;
  bool castling[4] = {true, true, true, true}; // KQkq
  Square enPassant = NO_SQ;
  int halfMoveClock = 0;
  int fullMove = 1;
};

class Board {
public:
  Piece squares[64];
  BoardState state;

  Board();
  void loadFEN(const std::string &fen);
  std::string toFEN() const;

  Piece at(int rank, int file) const { return squares[rank * 8 + file]; };
  Piece &at(int rank, int file) { return squares[rank * 8 + file]; };
  Piece at(Square sq) const { return squares[sq]; };
  Piece &at(Square sq) { return squares[sq]; };

  void applyMove(const Move &m);
  void undoMove(const Move &m, const BoardState &prevState, Piece captured);

  void print() const;
};
