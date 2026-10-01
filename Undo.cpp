#include "Undo.hpp"

Move Undo::getMove() const {
    return move;
}

int Undo::getCastlePerm() const {
    return castlePerm;
}

int Undo::getEnPass() const {
    return enPass;
}

U64 Undo::getPosKey() const {
    return posKey;
}

int Undo::getFiftyMove() const {
    return fiftyMove; 
}


void Undo::setPosKey(U64 newPosKey){
    posKey = newPosKey; 
}

void Undo::setMove(Move newMove){
    move = newMove; 
}

void Undo::setFiftyMove(int newFiftyMove){
    fiftyMove = newFiftyMove; 
}

void Undo::setCastlePerm(int newCastlePerm){
    castlePerm = newCastlePerm;
}
void Undo::setEnPass(int newEnpass){
    enPass = newEnpass; 
}