#include "board.h"
#include "movegen.h"
#include <iostream>

int main() {
  Board b;
  b.print();

  auto moves = generateMoves(b);
  std::cout << "Legal moves from start: " << moves.size() << "\n";
  // Should be exactly 20 (16 pawn + 4 knight moves)

  return 0;
}
