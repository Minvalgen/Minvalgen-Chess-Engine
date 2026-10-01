#include "Defs.h"

int SqOnBoard(int sq){
    return ((FileBrd[sq] == OFF_BOARD) ? 0 : 1);
}

int SideValid(int side){
    return ((side == WHITE || side == BLACK) ? 1 : 0);
}
int FileRankValid(int fr){
    return (fr >= 0 && fr <= 7);
}
int PieceValid(int piece){
    return (((piece >= wP) && (piece <= bK)) ? 1 : 0);
}
int PieceEmptyValid(int piece){
    return ((((piece >= wP) && (piece <= bK))  || piece == EMPTY) ? 1 : 0);
}