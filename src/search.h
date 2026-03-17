#pragma once
#include "board.h"
#include "move.h"

struct SearchResult {
  Move bestMove;
  int score;
};

SearchResult search(Board &b, int depth);
