#pragma once 

#include "Defs.h"
#include <iostream>
#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "MakeMove.hpp"
#include "Move.hpp"

class Perft
{
    private:
        Board *pos; 
        long long leaveNodes = 0; 
        long long pref = 0; 
    public:
        Perft(Board *pos);
        void PerfTest(int depth);
        long long getNumMoves();

};

