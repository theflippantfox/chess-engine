#include "board.h"
#include "move.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

Board::Board() {
  loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::loadFEN(const std::string &fen) {
  for (auto &sq : squares)
    sq = EMPTY;

  std::istringstream ss(fen);
  std::string piecePlacement, side, castle, ep;
  ss >> piecePlacement >> side >> castle >> ep >> state.halfMoveClock >>
      state.fullMove;

  // Parse piece placement (rank 8 down to rank 1)
  int rank = 7, file = 0;
  for (char c : piecePlacement) {
    if (c == '/') {
      --rank;
      file = 0;
    } else if (c >= '1' && c <= '8')
      file += (c - '0');
    else {
      // Map FEN char to Piece
      static const std::string fenChars = "PNBRQKpnbrqk";
      static const Piece fenPieces[] = {W_PAWN,   W_KNIGHT, W_BISHOP, W_ROOK,
                                        W_QUEEN,  W_KING,   B_PAWN,   B_KNIGHT,
                                        B_BISHOP, B_ROOK,   B_QUEEN,  B_KING};
      size_t idx = fenChars.find(c);
      if (idx != std::string::npos)
        at(rank, file++) = fenPieces[idx];
    }
  }

  state.sideToMove = (side == "w") ? WHITE : BLACK;
  state.castling[0] = castle.find("K") != std::string::npos; // white kingside
  state.castling[1] = castle.find("Q") != std::string::npos; // white queenside
  state.castling[2] = castle.find("k") != std::string::npos; // black kingside
  state.castling[3] = castle.find("q") != std::string::npos; // black queenside

  state.enPassant = (ep == "-") ? NO_SQ : (ep[1] - '1') * 8 + (ep[0] - 'a');
}

void Board::print() const {
  static const char *glyphs[] = {".", "P", "N", "B", "R", "Q", "K", ".",
                                 ".", "p", "n", "b", "r", "q", "k"};
  std::cout << "\n  a b c d e f g h\n";
  for (int r = 7; r >= 0; --r) {
    std::cout << (r + 1) << " ";
    for (int f = 0; f < 8; ++f)
      std::cout << glyphs[at(r, f)] << " ";
    std::cout << (r + 1) << "\n";
  }
  std::cout << "  a b c d e f g h\n\n";
  std::cout << (state.sideToMove == WHITE ? "White" : "Black") << " to move\n";
}

void Board::applyMove(const Move &m) {
  Piece moving = at(m.from);

  // Handling En Passant
  if (moving == W_PAWN && m.to == state.enPassant)
    at(m.to - 8) = EMPTY;
  if (moving == B_PAWN && m.to == state.enPassant)
    at(m.to + 8) = EMPTY;

  // Handling Castling
  if (moving == W_KING && m.from == 4) {
    if (m.to == 6) {
      at(7) = EMPTY;
      at(5) = W_ROOK;
    }
    if (m.to == 2) {
      at(0) = EMPTY;
      at(3) = W_ROOK;
    }
  }

  if (moving == B_KING && m.from == 60) {
    if (m.to == 62) {
      at(63) = EMPTY;
      at(61) = B_ROOK;
    }
    if (m.to == 58) {
      at(56) = EMPTY;
      at(59) = B_ROOK;
    }
  }

  // Handling Promotion
  at(m.to) = (m.promotion != EMPTY) ? m.promotion : moving;
  at(m.from) = EMPTY;

  // Update en passant square for next move
  state.enPassant = NO_SQ;
  if (moving == W_PAWN && m.to - m.from == 16)
    state.enPassant = m.from + 8;
  if (moving == B_PAWN && m.from - m.to == 16)
    state.enPassant = m.from - 8;

  // Update castling rights
  if (moving == W_KING) {
    state.castling[0] = state.castling[1] = false;
  }
  if (moving == B_KING) {
    state.castling[2] = state.castling[3] = false;
  }
  if (m.from == 0 || m.to == 0) {
    state.castling[1] = false;
  }
  if (m.from == 7 || m.to == 7) {
    state.castling[0] = false;
  }
  if (m.from == 56 || m.to == 56) {
    state.castling[3] = false;
  }
  if (m.from == 63 || m.to == 63) {
    state.castling[2] = false;
  }

  state.sideToMove = (state.sideToMove == WHITE) ? BLACK : WHITE;
  state.fullMove += (state.sideToMove == WHITE) ? 1 : 0;
}

void Board::undoMove(const Move &m, const BoardState &prevState,
                     Piece captured) {
  Piece moving = at(m.to);

  // Undo promotion: restore pawn
  if (m.promotion != EMPTY)
    moving = (prevState.sideToMove == WHITE) ? W_PAWN : B_PAWN;

  at(m.from) = moving;
  at(m.to) = captured;

  // Undo en passant capture
  if (moving == W_PAWN && m.to == prevState.enPassant)
    at(m.to - 8) = B_PAWN;
  if (moving == B_PAWN && m.to == prevState.enPassant)
    at(m.to + 8) = W_PAWN;

  // Undo castling rook move
  if (moving == W_KING && m.from == 4) {
    if (m.to == 6) {
      at(7) = W_ROOK;
      at(5) = EMPTY;
    }
    if (m.to == 2) {
      at(0) = W_ROOK;
      at(3) = EMPTY;
    }
  }
  if (moving == B_KING && m.from == 60) {
    if (m.to == 62) {
      at(63) = B_ROOK;
      at(61) = EMPTY;
    }
    if (m.to == 58) {
      at(56) = B_ROOK;
      at(59) = EMPTY;
    }
  }

  state = prevState; // restore ALL state
}
