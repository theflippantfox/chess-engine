#include "board.h"
#include "movegen.h"
#include "search.h"
#include <iostream>

int main() {
  Board b;
  b.print();

  std::cout << "Searching at depth 4...\n";
  auto result = search(b, 4);

  int from = result.bestMove.from;
  int to = result.bestMove.to;
  std::cout << "Best move: " << (char)('a' + from % 8) << (from / 8 + 1)
            << (char)('a' + to % 8) << (to / 8 + 1)
            << "  score: " << result.score << "\n";
  return 0;
}
