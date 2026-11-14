CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
SRC      = $(wildcard src/*.cpp)
OBJ      = $(SRC:.cpp=.o)
BIN      = bdsp

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp src/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)
	rm -rf salidas

.PHONY: clean
