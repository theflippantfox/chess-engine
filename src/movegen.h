#pragma once
#include "board.h"
#include "move.h"
#include <vector>

// Returns all pseudo-legal moves for the side to move.
std::vector<Move> generateMoves(const Board &b);

// Returns true if the given side's king is currently in check.
bool isInCheck(const Board &b, Color side);
