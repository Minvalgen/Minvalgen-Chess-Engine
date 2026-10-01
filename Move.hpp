#pragma once
#include "Defs.h"


#define CAP 0x7c000
#define PAWNSTART 0x80000
#define ENPASS 0x40000
#define PRO 0xf00000
#define CAST 0x1000000
class Move
{
    private:
        int move;
        int score; 
    public:
        int FromSq() const ;
        int ToSq() const ;
        int Captured() const;
        int Promoted() const;
        bool IsEp() const;
        bool IsPs() const;
        bool IsCs() const;
        bool IsCap() const;
        bool IsPro() const;
        void setMove(int move);
        void setScore(int score);
        int getMove() const;
        int getScore() const;

        void PrintMove();
        void PrtAgeMove();
        std::string GetMoveString() const;
};