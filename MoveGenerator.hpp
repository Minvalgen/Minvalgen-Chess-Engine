#pragma once

#include "Defs.h"
#include "Board.hpp"
#include "Move.hpp"

#define MOVE(f , t , cap , pro , fl) (f | (t << 7) | (cap << 14) | (pro << 20) | fl)
#define SQOFBOARD(sq) (FileBrd[sq] == OFF_BOARD)


class MoveGenerator
{
    private:
    int countMoves;
    const Board *pos;
    Move moves[MAXPOSMOVES];
    void AddQmove( int move);
    void AddCapMove(int move);
    void AddEnPasMove (int move);
    void AddPawnCapMove( int from , int to , int cap , int side);
    void AddPawnMove(int from , int to , int side);
    void GenPawnMoves(int side);
    void GenSlideMoves(int side);
    void GenNonSlideMoves(int side);
    void GenCastlMoves(int side);
    public:
        void GenerateAllMoves();
        void GenerateCaptures();
        void PrintMoves();

        MoveGenerator(Board *_pos);
        int getCountMoves();
        Move getMove(int index);
};

