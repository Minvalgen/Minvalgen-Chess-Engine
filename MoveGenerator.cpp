#include "MoveGenerator.hpp"
#include <cassert>
#include <iostream>

MoveGenerator :: MoveGenerator(Board *_pos){
    this->pos = _pos;
}

int MoveGenerator:: getCountMoves(){
    return countMoves; 
}

Move MoveGenerator::getMove(int index){
    return moves[index];
}
void MoveGenerator::AddQmove(int move){
    moves[countMoves].setMove(move);
    moves[countMoves].setScore(0);
    countMoves++;
}

void MoveGenerator::AddCapMove(int move){
    moves[countMoves].setMove(move);
    moves[countMoves].setScore(0);
    countMoves++;
}

void MoveGenerator::AddEnPasMove(int move){
    moves[countMoves].setMove(move);
    moves[countMoves].setScore(0);
    countMoves++;
}

void MoveGenerator::AddPawnCapMove(int from , int to , int cap , int side){

    /* -- checks -- */
    assert(SqOnBoard(from));
    assert(SqOnBoard(to));
    assert(PieceEmptyValid(cap));

    int rank = ((side == WHITE) ? RANK_7 : RANK_2);
    int start = ((side == WHITE) ? wN : bN);
    int end = ((side == WHITE) ? wQ : bQ);
    if (RankBrd[from] == rank){
        for(int pro = start ; pro <= end ; pro++){
            
            AddCapMove(MOVE(from , to , cap , pro , 0));
        }
    }
    else{
        AddCapMove(MOVE(from , to , cap , EMPTY , 0));
    }
}

void MoveGenerator::AddPawnMove(int from , int to , int side){ 
    /* -- checks --*/
    assert(SqOnBoard(from));
    assert(SqOnBoard(to));

    int rank = ((side == WHITE) ? RANK_7 : RANK_2);
    int start = ((side == WHITE) ? wN : bN);
    int end = ((side == WHITE) ? wQ : bQ);
    if (RankBrd[from] == rank){
        for(int pro = start ; pro <= end ; pro++){
         AddQmove(MOVE(from , to , EMPTY , pro , 0));
        }
    }
    else{
        AddQmove(MOVE(from , to , EMPTY , EMPTY , 0));
    }
}


void MoveGenerator::GenPawnMoves(int side){ // this function generates the moves of the panws 

    int sq; 
    int pwan = ((side == WHITE )? wP : bP);
    int color = ((side == WHITE) ? BLACK : WHITE);
    int d = ((side == WHITE) ? 1 : -1);
    int rank = ((side == WHITE) ? RANK_2 : RANK_7);

    for(int pceNum = 0 ; pceNum < pos->getPceNum(pwan) ; pceNum++){
            sq = pos->getPceSq(pwan , pceNum);
            assert(SqOnBoard(sq));
            if (pos->getPieceOnSq(sq + d*10) == EMPTY){
                AddPawnMove(sq , sq + d * 10 , side);
                if ((RankBrd[sq] == rank) && (pos->getPieceOnSq(sq + d*20) == EMPTY)){
                    AddQmove(MOVE(sq , sq + d * 20 , EMPTY , EMPTY, PAWNSTART));
                }
            }
            for(int m = 9 ; m <= 11 ; m+=2){
                if (!SQOFBOARD(sq + d * m) && (PicColor[pos->getPieceOnSq(sq + d * m)] == color)){
                    AddPawnCapMove(sq , sq + d * m , pos->getPieceOnSq(sq + d * m) , side);
                }
                if ((sq + d * m) == pos->getEnPass()){
                    AddCapMove( MOVE(sq , sq + d * m , EMPTY , EMPTY , ENPASS));
                }
            }
        }
}






void MoveGenerator::GenSlideMoves(int side){ // this function generate all sliding moves "bishop , queen , rook"
    int index = LoopSlideInxed[side];
    int pce = LoopSlidePieces[index++];
    int sq;
    int t_sq;
    while (pce != 0)
    {
        assert(PieceValid(pce));
        for(int pceNum = 0 ; pceNum < pos->getPceNum(pce) ; pceNum++){
            sq = pos->getPceSq(pce , pceNum);
            assert(SqOnBoard(sq));
            for(int dir = 0 ; dir < NumDir[pce] ; dir++){
                int x = PceDir[pce][dir];
                t_sq = sq + x;
                while (!SQOFBOARD(t_sq))
                {
                    /* code */
                    int cap = pos->getPieceOnSq(t_sq);
                    if (cap != EMPTY){
                        if ((PicColor[cap] == side) ^ 1){
                           AddCapMove(MOVE(sq , t_sq , cap , EMPTY , 0));
                        }
                        break;
                    }
                    AddQmove(MOVE(sq , t_sq , EMPTY , EMPTY , 0));
                    t_sq += x;
                }
                


            }
        }
        pce = LoopSlidePieces[index++];

    }
}

void MoveGenerator::GenNonSlideMoves(int side){ // this function generate all non-sliding moves "knight , king"
    int index = LoopNonSlideInxed[side];
    int pce = LoopNonSlidePieces[index++];
    int sq;
    int t_sq; 
    while (pce != 0)
    {
        assert(PieceValid(pce));
        for(int pceNum = 0 ; pceNum < pos->getPceNum(pce) ; pceNum++){
            sq = pos->getPceSq(pce , pceNum);
            assert(SqOnBoard(sq));
            for(int dir = 0 ; dir < NumDir[pce] ; dir++){
                t_sq = sq + PceDir[pce][dir];
                if (SQOFBOARD(t_sq)){
                    continue;
                } 

                int cap = pos->getPieceOnSq(t_sq);
                if (cap != EMPTY){
                    if ((PicColor[cap] == side) ^ 1){
                        AddCapMove(MOVE(sq , t_sq , cap , EMPTY , 0));
                        
                    }
                    continue;
                }
                AddQmove(MOVE(sq , t_sq , EMPTY , EMPTY , 0));
                

            }
        }
        pce = LoopNonSlidePieces[index++];
    }
}

void MoveGenerator::GenCastlMoves(int side){
    int start1 = ((side == WHITE) ? F1 : F8);
    int end1 = ((side == WHITE) ? G1 : G8);

    int start2 = ((side == WHITE) ? B1 : B8);
 
    int fromK = ((side == WHITE) ? E1 : E8);
    int toK = ((side == WHITE) ? G1 : G8);

    int fromQ = ((side == WHITE) ? E1 : E8);
    int toQ = ((side == WHITE) ? C1 : C8);

    if (pos->IsKingSideCastl(side)){
        if (pos->IsAttacked(fromK, side ^ 1) == FALSE) {
            int ok = 1;
            for(int i = start1 ; i <= end1 ; i++){
                ok &= ((pos->getPieceOnSq(i) == EMPTY) && (pos->IsAttacked(i , side ^ 1) == FALSE));
            }
            if (ok == TRUE){
                AddQmove(MOVE(fromK , toK , EMPTY , EMPTY , CAST));
            }
        }
    }

    if (pos->IsQueenSideCastl(side)){
         if (pos->IsAttacked(fromQ, side ^ 1) == FALSE) {
             if (pos->getPieceOnSq(start2) == EMPTY && 
                 pos->getPieceOnSq(start2 + 1) == EMPTY && 
                 pos->getPieceOnSq(start2 + 2) == EMPTY) {
                
                if (pos->IsAttacked(start2 + 1, side ^ 1) == FALSE && 
                    pos->IsAttacked(start2 + 2, side ^ 1) == FALSE) {
                    AddQmove(MOVE(fromQ , toQ , EMPTY , EMPTY , CAST));
                }
             }
         }
    }
}

void MoveGenerator::GenerateAllMoves(){
    countMoves = 0;
    int side = this->pos->getSide();
    GenPawnMoves(side);
    GenSlideMoves(side);
    GenNonSlideMoves(side);
    GenCastlMoves(side);
}

void MoveGenerator::GenerateCaptures(){
    countMoves = 0;
    int side = this->pos->getSide();
    GenPawnMoves(side);
    GenSlideMoves(side);
    GenNonSlideMoves(side);
    
    int writeIndex = 0;
    for (int i = 0; i < countMoves; i++) {
        if (moves[i].Captured() != EMPTY || moves[i].Promoted() != EMPTY || moves[i].IsEp()) {
            moves[writeIndex++] = moves[i];
        }
    }
    countMoves = writeIndex;
}


void MoveGenerator::PrintMoves(){
    printf("Moves of the current position : \n");
    int n = countMoves;
    for(int i = 0 ; i < n ; i++){
        int score = moves[i].getScore();
        printf("move : %d " , i);
        moves[i].PrtAgeMove();
    }
}