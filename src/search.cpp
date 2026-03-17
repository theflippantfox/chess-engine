#include "search.h"
#include "eval.h"
#include "movegen.h"
#include <limits>

static const int INF = std::numeric_limits<int>::max() / 2;

// Alpha: best score White is guaranteed so far
// Beta:  best score Black is guaranteed so far
// If alpha >= beta, the opponent won't let us reach this node → prune
static int alphaBeta(Board &b, int depth, int alpha, int beta) {
  if (depth == 0)
    return evaluate(b);

  auto moves = generateMoves(b);

  // No legal moves = checkmate or stalemate
  if (moves.empty()) {
    if (isInCheck(b, b.state.sideToMove))
      return (b.state.sideToMove == WHITE) ? -INF : INF; // checkmate
    return 0;                                            // stalemate
  }

  if (b.state.sideToMove == WHITE) {
    int best = -INF;
    for (const Move &m : moves) {
      BoardState prev = b.state;
      Piece captured = b.at(m.to);
      b.applyMove(m);
      best = std::max(best, alphaBeta(b, depth - 1, alpha, beta));
      b.undoMove(m, prev, captured);
      alpha = std::max(alpha, best);
      if (alpha >= beta)
        break; // ← beta cutoff, prune!
    }
    return best;
  } else {
    int best = INF;
    for (const Move &m : moves) {
      BoardState prev = b.state;
      Piece captured = b.at(m.to);
      b.applyMove(m);
      best = std::min(best, alphaBeta(b, depth - 1, alpha, beta));
      b.undoMove(m, prev, captured);
      beta = std::min(beta, best);
      if (alpha >= beta)
        break; // ← alpha cutoff, prune!
    }
    return best;
  }
}

SearchResult search(Board &b, int depth) {
  int alpha = -INF, beta = INF;
  int bestScore = (b.state.sideToMove == WHITE) ? -INF : INF;
  Move bestMove;
  Color us = b.state.sideToMove;

  for (const Move &m : generateMoves(b)) {
    BoardState prev = b.state;
    Piece captured = b.at(m.to);
    b.applyMove(m);
    int score = alphaBeta(b, depth - 1, alpha, beta);
    b.undoMove(m, prev, captured);

    if (us == WHITE ? score > bestScore : score < bestScore) {
      bestScore = score;
      bestMove = m;
    }
    if (us == WHITE)
      alpha = std::max(alpha, bestScore);
    else
      beta = std::min(beta, bestScore);
  }
  return {bestMove, bestScore};
}
