#pragma once

#include "Defs.h"
#include "Board.hpp"
#include "Move.hpp"
#include <cassert>


class MakeMove
{
    private:
        Board *pos;
        void ClearPiece(const int sq);
        void AddPiece(const int sq , const int pce);
        void MovePiece(const int from , const int to);
        
        public:
        MakeMove(Board *pos);
        int MakeMoves(Move move);
        void TakeMove();
        void MakeNullMove();
        void TakeNullMove();
};


