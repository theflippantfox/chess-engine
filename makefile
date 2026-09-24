CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = src/main.cpp src/board.cpp src/movegen.cpp src/search.cpp src/eval.cpp
OUT = build/

all:
	mkdir -p build/
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)/chess
	cp -r web/* build/
	cd server && npm install

run-web: all
	cd server && npm start

clean:
	rm -rf $(OUT)
