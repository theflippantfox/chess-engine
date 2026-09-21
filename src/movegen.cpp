#include "movegen.h"
#include "board.h"
#include "move.h"
#include <algorithm>
#include <array>
#include <utility>
#include <vector>

static inline int rankOf(Square sq) { return sq / 8; }
static inline int fileOf(Square sq) { return sq % 8; }
static inline Square makeSquare(int r, int f) { return r * 8 + f; }
static inline bool onBoard(int r, int f) {
  return r >= 0 && r < 8 && f >= 0 && f < 8;
}

// Sliding Pieces (Bishop, Queen, Rook)
template <size_t N>
static void addSlidingMoves(const Board &b, Square from, Color us,
                            const std::array<std::pair<int, int>, N> &dirs,
                            std::vector<Move> &moves) {
  for (auto [dr, df] : dirs) {
    int r = rankOf(from) + dr, f = fileOf(from) + df;
    while (onBoard(r, f)) {
      Square to = makeSquare(r, f);
      Piece target = b.at(to);
      if (isEmpty(target)) {
        moves.emplace_back(from, to);
      } else {
        if (pieceColor(target) != us)
          moves.emplace_back(from, to);
        break;
      }
      r += dr;
      f += df;
    }
  }
}

// Pawns
static void addPawnMoves(const Board &b, Square from, Color us,
                         std::vector<Move> &moves) {
  int dir = (us == WHITE) ? 1 : -1;
  int startRank = (us == WHITE) ? 1 : 6;
  int promoRank = (us == WHITE) ? 7 : 0;
  Piece promos[] = {
      (us == WHITE) ? W_QUEEN : B_QUEEN, (us == WHITE) ? W_ROOK : B_ROOK,
      (us == WHITE) ? W_BISHOP : B_BISHOP, (us == WHITE) ? W_KNIGHT : B_KNIGHT};

  int r = rankOf(from), f = fileOf(from);

  // One step forward
  if (onBoard(r + dir, f) && isEmpty(b.at(makeSquare(r + dir, f)))) {
    if (r + dir == promoRank)
      for (Piece p : promos)
        moves.emplace_back(from, makeSquare(r + dir, f), p);
    else {
      moves.emplace_back(from, makeSquare(r + dir, f));
      if (r == startRank && isEmpty(b.at(makeSquare(r + 2 * dir, f))))
        moves.emplace_back(from, makeSquare(r + 2 * dir, f));
    }
  }

  // Captures (including en passant)
  for (int df : {-1, 1}) {
    if (!onBoard(r + dir, f + df))
      continue;
    Square to = makeSquare(r + dir, f + df);
    Piece target = b.at(to);
    bool isEP = (to == b.state.enPassant);
    if ((!isEmpty(target) && pieceColor(target) != us) || isEP) {
      if (r + dir == promoRank)
        for (Piece p : promos)
          moves.emplace_back(from, to, p);
      else
        moves.emplace_back(from, to);
    }
  }
}

// Knight
static void addKnightMoves(const Board &b, Square from, Color us,
                           std::vector<Move> &moves) {
  static const std::array<std::pair<int, int>, 8> hops = {
      {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}}};
  for (auto [dr, df] : hops) {
    int r = rankOf(from) + dr, f = fileOf(from) + df;
    if (!onBoard(r, f))
      continue;
    Square to = makeSquare(r, f);
    if (!isFriendly(b.at(to), us))
      moves.emplace_back(from, to);
  }
}

// King
static void addKingMoves(const Board &b, Square from, Color us,
                         std::vector<Move> &moves) {
  static const std::array<std::pair<int, int>, 8> dirs = {
      {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
  for (auto [dr, df] : dirs) {
    int r = rankOf(from) + dr, f = fileOf(from) + df;
    if (!onBoard(r, f))
      continue;
    Square to = makeSquare(r, f);
    if (isEmpty(b.at(to)) || pieceColor(b.at(to)) != us)
      moves.emplace_back(from, to);
  }

  // Castling
  if (us == WHITE && from == 4) {
    if (b.state.castling[0] && isEmpty(b.at(5)) && isEmpty(b.at(6)))
      moves.emplace_back(4, 6); // kingside
    if (b.state.castling[1] && isEmpty(b.at(3)) && isEmpty(b.at(2)) &&
        isEmpty(b.at(1)))
      moves.emplace_back(4, 2); // queenside
  }
  if (us == BLACK && from == 60) {
    if (b.state.castling[2] && isEmpty(b.at(61)) && isEmpty(b.at(62)))
      moves.emplace_back(60, 62); // kingside
    if (b.state.castling[3] && isEmpty(b.at(59)) && isEmpty(b.at(58)) &&
        isEmpty(b.at(57)))
      moves.emplace_back(60, 58); // queenside
  }
}

// Is In Check?
bool isInCheck(const Board &b, Color side) {
  // Find the king
  Piece king = (side == WHITE) ? W_KING : B_KING;
  Square kingSq = NO_SQ;
  for (Square sq = 0; sq < 64; ++sq)
    if (b.at(sq) == king) {
      kingSq = sq;
      break;
    }
  if (kingSq == NO_SQ)
    return false;

  Color opp = (side == WHITE) ? BLACK : WHITE;
  int kr = rankOf(kingSq), kf = fileOf(kingSq);

  // Attacked by knight?
  for (auto [dr, df] : std::array<std::pair<int, int>, 8>{{{2, 1},
                                                           {2, -1},
                                                           {-2, 1},
                                                           {-2, -1},
                                                           {1, 2},
                                                           {1, -2},
                                                           {-1, 2},
                                                           {-1, -2}}}) {
    int r = kr + dr, f = kf + df;
    if (onBoard(r, f)) {
      Piece p = b.at(makeSquare(r, f));
      if (p == (opp == WHITE ? W_KNIGHT : B_KNIGHT))
        return true;
    }
  }

  // Attacked along ranks/files (rook or queen)?
  for (auto [dr, df] :
       std::array<std::pair<int, int>, 4>{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}}) {
    int r = kr + dr, f = kf + df;
    while (onBoard(r, f)) {
      Piece p = b.at(makeSquare(r, f));
      if (!isEmpty(p)) {
        if (pieceColor(p) == opp && (p == (opp == WHITE ? W_ROOK : B_ROOK) ||
                                     p == (opp == WHITE ? W_QUEEN : B_QUEEN)))
          return true;
        break;
      }
      r += dr;
      f += df;
    }
  }

  // Attacked diagonally (bishop or queen)?
  for (auto [dr, df] : std::array<std::pair<int, int>, 4>{
           {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}}) {
    int r = kr + dr, f = kf + df;
    while (onBoard(r, f)) {
      Piece p = b.at(makeSquare(r, f));
      if (!isEmpty(p)) {
        if (pieceColor(p) == opp &&
            (p == (opp == WHITE ? W_BISHOP : B_BISHOP) ||
             p == (opp == WHITE ? W_QUEEN : B_QUEEN)))
          return true;
        break;
      }
      r += dr;
      f += df;
    }
  }

  // Attacked by pawn?
  int pawnDir = (side == WHITE) ? 1 : -1;
  for (int df : {-1, 1}) {
    int r = kr + pawnDir, f = kf + df;
    if (onBoard(r, f)) {
      Piece p = b.at(makeSquare(r, f));
      if (p == (opp == WHITE ? W_PAWN : B_PAWN))
        return true;
    }
  }

  // Attacked by king? (needed to avoid illegal king moves)
  for (auto [dr, df] : std::array<std::pair<int, int>, 8>{{{1, 0},
                                                           {-1, 0},
                                                           {0, 1},
                                                           {0, -1},
                                                           {1, 1},
                                                           {1, -1},
                                                           {-1, 1},
                                                           {-1, -1}}}) {
    int r = kr + dr, f = kf + df;
    if (onBoard(r, f)) {
      Piece p = b.at(makeSquare(r, f));
      if (p == (opp == WHITE ? W_KING : B_KING))
        return true;
    }
  }

  return false;
}

// Main Entry Point
std::vector<Move> generateMoves(const Board &b) {
  std::vector<Move> moves;
  moves.reserve(64);
  Color us = b.state.sideToMove;

  static const std::array<std::pair<int, int>, 4> rookDirs = {
      {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
  static const std::array<std::pair<int, int>, 4> bishopDirs = {
      {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
  static const std::array<std::pair<int, int>, 8> queenDirs = {
      {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};

  for (Square sq = 0; sq < 64; ++sq) {
    Piece p = b.at(sq);

    if (isEmpty(p))
      continue;
    if (us == WHITE && !isWhitePiece(p))
      continue;
    if (us == BLACK && !isBlackPiece(p))
      continue;

    switch (p) {
    case W_PAWN:
    case B_PAWN:
      addPawnMoves(b, sq, us, moves);
      break;
    case W_KNIGHT:
    case B_KNIGHT:
      addKnightMoves(b, sq, us, moves);
      break;
    case W_BISHOP:
    case B_BISHOP:
      addSlidingMoves(b, sq, us, bishopDirs, moves);
      break;
    case W_ROOK:
    case B_ROOK:
      addSlidingMoves(b, sq, us, rookDirs, moves);
      break;
    case W_QUEEN:
    case B_QUEEN:
      addSlidingMoves(b, sq, us, queenDirs, moves);
      break;
    case W_KING:
    case B_KING:
      addKingMoves(b, sq, us, moves);
      break;
    default:
      break;
    }
  }

  // Filter pseudo-legal → legal: remove moves that leave our king in check
  moves.erase(std::remove_if(moves.begin(), moves.end(),
                             [&](const Move &m) {
                               Board tmp = b;
                               tmp.applyMove(m);
                               return isInCheck(tmp, us);
                             }),
              moves.end());

  return moves;
}
