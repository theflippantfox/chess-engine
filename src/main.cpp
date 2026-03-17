#include "board.h"
#include "search.h"
#include <iostream>
#include <sstream>
#include <string>

static Move parseMove(const std::string &s, const Board &b) {
  Square from = (s[1] - '1') * 8 + (s[0] - 'a');
  Square to = (s[3] - '1') * 8 + (s[2] - 'a');

  Piece promo = EMPTY;
  if (s.size() == 5) {
    switch (s[4]) {
    case 'q':
      promo = (b.state.sideToMove == WHITE) ? W_QUEEN : B_QUEEN;
      break;
    case 'r':
      promo = (b.state.sideToMove == WHITE) ? W_ROOK : B_ROOK;
      break;
    case 'b':
      promo = (b.state.sideToMove == WHITE) ? W_BISHOP : B_BISHOP;
      break;
    case 'n':
      promo = (b.state.sideToMove == WHITE) ? W_KNIGHT : B_KNIGHT;
      break;
    }
  }
  return Move(from, to, promo);
}

static std::string moveToString(const Move &m) {
  std::string s;
  s += (char)('a' + m.from % 8);
  s += (char)('1' + m.from / 8);
  s += (char)('a' + m.to % 8);
  s += (char)('1' + m.to / 8);
  if (m.promotion != EMPTY) {
    const char *promoChars = ".pnbrqk..pnbrqk";
    s += promoChars[m.promotion];
  }
  return s;
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  Board board;
  std::string line;

  while (std::getline(std::cin, line)) {
    // Strip Windows-style \r if present
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    // Debug: log every line received
    std::cerr << "[in]  " << line << "\n";
    std::cerr.flush();

    if (line.empty())
      continue;

    std::istringstream ss(line);
    std::string token;
    ss >> token;

    if (token == "uci") {
      std::cout << "id name ChessEngine" << std::endl;
      std::cout << "id author You" << std::endl;
      std::cout << "uciok" << std::endl;
      std::cerr << "[out] uciok\n";

    } else if (token == "isready") {
      std::cout << "readyok" << std::endl;
      std::cerr << "[out] readyok\n";

    } else if (token == "ucinewgame") {
      board = Board();

    } else if (token == "position") {
      ss >> token;
      if (token == "startpos") {
        board = Board();
        ss >> token; // consume "moves" if present
      } else if (token == "fen") {
        std::string fen, part;
        for (int i = 0; i < 6; ++i) {
          ss >> part;
          if (i > 0)
            fen += ' ';
          fen += part;
        }
        board.loadFEN(fen);
        ss >> token; // consume "moves" if present
      }
      if (token == "moves") {
        std::string mv;
        while (ss >> mv)
          board.applyMove(parseMove(mv, board));
      }

    } else if (token == "go") {
      int depth = 4;
      std::string param;
      while (ss >> param) {
        if (param == "depth") {
          ss >> param;
          depth = std::stoi(param);
        }
        // ignore wtime, btime, movetime etc.
      }
      auto result = search(board, depth);
      std::string best = moveToString(result.bestMove);
      std::cout << "bestmove " << best << std::endl;
      std::cerr << "[out] bestmove " << best << "\n";

    } else if (token == "d") {
      board.print();

    } else if (token == "quit") {
      break;
    }

    std::cerr.flush();
  }
  return 0;
}
