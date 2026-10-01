#pragma once

#include "Defs.h"
#include "Board.hpp"
#include "Search.hpp"
#include "MakeMove.hpp"
#include "MoveGenerator.hpp"
#include <string>

class UCI {
private:
    Board board;

    void ParsePosition(const std::string& line);
    void ParseGo(const std::string& line);
    Move ParseMoveString(const std::string& moveStr);
    std::string GetBookMove(const std::string& moves);

    std::string currentMoves;

public:
    UCI() = default;
    void Loop();
};
