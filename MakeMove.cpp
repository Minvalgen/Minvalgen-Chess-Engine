#include "MakeMove.hpp"


MakeMove::MakeMove(Board *_pos){
    this->pos = _pos;
}

void MakeMove::ClearPiece(const int sq){
    assert(SqOnBoard(sq));
    int pce = this->pos->getPieceOnSq(sq);
    assert(PieceValid(pce));

    int color = PicColor[pce];
    int t_pceNum = -1; 
    pos->HashPiece(pce , sq);
    
    pos->setPieceOnSq(sq , EMPTY);
    pos->setMaterial(color , sq ,-1);
    
    if (PicBig[pce] == TRUE){
        pos->setBig(color , -1);
        if (PicMajor[pce] == TRUE){
            pos->setMajor(color , -1);
        }
        else{
            pos->setMinor(color , -1);
        }
    }
    else{
        pos->clearBit(color , sq);
        pos->clearBit(BOTH , sq);
    }

    for(int i = 0; i < pos->getPceNum(pce) ; i++){
        if (pos->getPceSq(pce , i) == sq){
            t_pceNum = i;
            break;
        }
    }
    assert(t_pceNum != -1);
    pos->setPieceNum(pce , -1);
    int nSQ = pos->getPceSq(pce , pos->getPceNum(pce));
    pos->setPieceList(pce , t_pceNum, nSQ);

}

void MakeMove::AddPiece(const int sq , const int pce){

    assert(SqOnBoard(sq));
    assert(PieceValid(pce));
    int color = PicColor[pce];
    pos->HashPiece(pce , sq);

    
    pos->setPieceOnSq(sq , pce);
    pos->setMaterial(color , sq ,1);

    if (PicBig[pce] == TRUE){
        pos->setBig(color , 1);
        if (PicMajor[pce] == TRUE){
            pos->setMajor(color , 1);
        }
        else{
            pos->setMinor(color , 1);
        }
    }
    else{
            pos->setBit(color, sq);
            pos->setBit(BOTH, sq);  
    }

    pos->setPieceList(pce , pos->getPceNum(pce) , sq);
    pos->setPieceNum(pce , 1);

}

void MakeMove::MovePiece(const int from , const int to){
    assert(SqOnBoard(from));
    assert(SqOnBoard(to));

    int pce = pos->getPieceOnSq(from);
    int color = PicColor[pce];

    pos->HashPiece(pce , from);
    pos->setPieceOnSq(from, EMPTY);

    pos->HashPiece(pce , to);
    pos->setPieceOnSq(to , pce);


    if (PicBig[pce] == FALSE){
        pos->clearBit(color , from);
        pos->clearBit(BOTH , from);
        pos->setBit(color , to);
        pos->setBit(BOTH , to);
    }
    int flag = FALSE; 
    for(int i = 0 ; i < pos->getPceNum(pce) ; i++){
        if (pos->getPceSq(pce , i) == from){
            pos->setPieceList(pce , i , to);
            flag = TRUE;
            break;
        }
    }
    assert(flag);

}

int MakeMove::MakeMoves(Move move){
    int from = move.FromSq();
    int to = move.ToSq(); 
    if (!SqOnBoard(from) || !SqOnBoard(to)) return false;

    int side = pos->getSide(); 
    int pce = pos->getPieceOnSq(from);

    assert(SideValid(side));
    assert(PieceValid(pce));

    int curHisPly = pos->getHisPly(); 
    pos->history[curHisPly].setPosKey(pos->getPosKey());

    if (move.IsEp()){
        if (side == WHITE){
            ClearPiece(to - 10);
        }
        else{
            ClearPiece(to + 10);
        }

    }
    else if (move.IsCs()){
        switch (to)
        {
        case C1:
            MovePiece(A1 , D1);
            break;
        case C8:
            MovePiece(A8 , D8);
            break;
        case G1:
            MovePiece(H1 , F1);
            break;
        case G8: 
            MovePiece(H8 , F8);
            break;
        default:
            assert(false);
            break;
        }
    }

    if (pos->getEnPass() != NO_SQ) pos->HashEnPass();
    pos->HashCastle();

    pos->history[curHisPly].setMove(move);
    pos->history[curHisPly].setEnPass(pos->getEnPass());
    pos->history[curHisPly].setCastlePerm(pos->getCastlePerm());
    pos->history[curHisPly].setFiftyMove(pos->getFiftyMove());

    pos->setCastlePerm(CastleBoard[from] & pos->getCastlePerm());
    pos->setCastlePerm(CastleBoard[to] & pos->getCastlePerm());

    pos->setEnPass(NO_SQ);

    pos->HashCastle();

    int CapPce = move.Captured();
    pos->updateFiftyMove(1);
    if (CapPce != EMPTY){
        assert(PieceValid(CapPce));
        ClearPiece(to);
        pos->setFiftyMove(0);
    }

    pos->setHisPly(1);
    pos->setPly(1);

    if (PiecePawn[pos->getPieceOnSq(from)] == TRUE){
        pos->setFiftyMove(0);

        if (move.IsPs()){

            if (side == WHITE){
                pos->setEnPass(from + 10);
                assert(RankBrd[pos->getEnPass()] == RANK_3);
            }
            else{
                pos->setEnPass(from - 10);
                assert(RankBrd[pos->getEnPass()] == RANK_6);
            }

            pos->HashEnPass(); 
        }

    }

    MovePiece(from , to);

    int proPce = move.Promoted(); 
    if (proPce != EMPTY){
        assert(PieceValid(proPce) && !PiecePawn[proPce]);
        ClearPiece(to);
        AddPiece(to , proPce);
    }

    if (PieceKing[pos->getPieceOnSq(to)]){
        pos->setKingPos(side , to);
    }

    pos->ChangeSide(); 
    pos->HashSide(); 
    
    if (pos->IsAttacked(pos->getKingPos(side) , pos->getSide())){
        TakeMove();
        return false;
    }
    return true; 

}

void MakeMove::TakeMove(){
    pos->setHisPly(-1);
    pos->setPly(-1);

    Undo oldPos = pos->history[pos->getHisPly()];
    Move move = oldPos.getMove();
    int from = move.FromSq();
    int to = move.ToSq(); 

    assert(SqOnBoard(from));
    assert(SqOnBoard(to));

    if (pos->getEnPass() != NO_SQ) pos->HashEnPass(); 
    pos->HashCastle();

    
    pos->setCastlePerm(oldPos.getCastlePerm());
    pos->setEnPass(oldPos.getEnPass());
    pos->setFiftyMove(oldPos.getFiftyMove());
    
    if (pos->getEnPass() != NO_SQ) pos->HashEnPass();
    pos->HashCastle(); 
    pos->ChangeSide();
    
    pos->HashSide(); 

    if (move.IsEp()){
        if (pos->getSide() == WHITE){
            AddPiece(to - 10 , bP);
        }
        else{
            AddPiece(to + 10 , wP);
        }

    }
    else if (move.IsCs()){
        switch (to)
        {
            case C1: MovePiece(D1, A1); break;
            case C8: MovePiece(D8, A8); break;
            case G1: MovePiece(F1, H1); break;
            case G8: MovePiece(F8, H8); break;
            default: assert(FALSE); break;
        }
    }

    MovePiece(to , from);

    if (PieceKing[pos->getPieceOnSq(from)]){
        pos->setKingPos(pos->getSide() , from);
    }

    int capPce = move.Captured(); 
    if (capPce != EMPTY){
        assert(PieceValid(capPce));
        AddPiece(to , capPce);
    }

    int proPce = move.Promoted(); 
    if (proPce != EMPTY){
        assert(PieceValid(proPce) && !PiecePawn[proPce]);
        ClearPiece(from);
        AddPiece(from , ((PicColor[proPce]  == WHITE) ? wP : bP));
    }


}

void MakeMove::MakeNullMove() {
    int curHisPly = pos->getHisPly();
    pos->history[curHisPly].setPosKey(pos->getPosKey());
    Move dummyMove;
    pos->history[curHisPly].setMove(dummyMove);
    pos->history[curHisPly].setEnPass(pos->getEnPass());
    pos->history[curHisPly].setCastlePerm(pos->getCastlePerm());
    pos->history[curHisPly].setFiftyMove(pos->getFiftyMove());

    if (pos->getEnPass() != NO_SQ) pos->HashEnPass();

    pos->setEnPass(NO_SQ);
    pos->updateFiftyMove(1);
    pos->setHisPly(1);
    pos->setPly(1);

    pos->ChangeSide();
    pos->HashSide();
}

void MakeMove::TakeNullMove() {
    pos->setHisPly(-1);
    pos->setPly(-1);

    Undo oldPos = pos->history[pos->getHisPly()];

    if (pos->getEnPass() != NO_SQ) pos->HashEnPass();

    pos->setEnPass(oldPos.getEnPass());
    pos->setCastlePerm(oldPos.getCastlePerm());
    pos->setFiftyMove(oldPos.getFiftyMove());

    if (pos->getEnPass() != NO_SQ) pos->HashEnPass();

    pos->ChangeSide();
    pos->HashSide();
}