CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall

SRCS = main.cpp Init.cpp Board.cpp Data.c Move.cpp MoveGenerator.cpp Validations.cpp MakeMove.cpp Undo.cpp perft.cpp Evaluate.cpp TranspositionTable.cpp Search.cpp UCI.cpp
TARGET = Minvalgen

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	del /f /q $(TARGET).exe

.PHONY: all clean