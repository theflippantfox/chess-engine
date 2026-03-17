CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = src/main.cpp src/board.cpp src/movegen.cpp src/search.cpp src/eval.cpp
OUT = build/

all:
	mkdir build/
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)/chess

clean:
	rm -rf $(OUT)
