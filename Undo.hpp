#pragma once 

#include "Defs.h"
#include  "Move.hpp"


class Undo
{
    private:
        Move move; 
        int castlePerm;
        int enPass;
        int fiftyMove;
        U64 posKey;
    public:
        Move getMove() const;
        int getCastlePerm() const;
        int getEnPass() const;
        int getFiftyMove() const;
        U64 getPosKey() const;


        void setMove(Move newMove);
        void setPosKey(U64 newPosKey);
        void setFiftyMove(int newFiftyMove);
        void setCastlePerm(int newCastlePerm);
        void setEnPass(int newEnpass);
};

