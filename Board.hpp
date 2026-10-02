#pragma once

#include <string>
#include "Defs.h"
#include "Undo.hpp"
#include <iostream>
class Board
{
    private:
        std::string fenPos;
        void updateMatrial();
        void ResetBoard();
        U64 GenerateHashKey();
        
        int sideToMove; // clear 
        int fiftyMove; // fifty move rule 
        int enPass; // en pass move 
        int ply;
        int hisPly;
        int castlePerm;
        U64 posKey;
        
        int pieceNum[13]; // the number of each piece on the board. 
        int kingSq[2]; // the squre of the white and black king.
        
        U64 pawns[3]; // bitBoard of the white panws, black pawns, and both.
        
        int big[3]; // number of big pieces of the white, black, and both.
        int major[3];// number of major pieces of the white, black, and both.
        int minor[3];// number of minor pieces of the white, black, and both.
        int material[3];// number of matrial pieces of the white, black, and both.
        int pieces[BOARD_SQ_NUM]; // for each square, whice piece is on it. 
        int pieceList[13][10]; // for each piece, the square it is on it.
        void setPos(std::string fen);
    public:
        Undo history[MAX_GAME_MOVES];// the history of the game.

        Board();
        void PrintBoard();
        void PrintAttacked(int side);
        int Parse(std::string s);

        int getCastlePerm() const ;
        U64 getPosKey() const ;
        int getBigNumber(int color) const;
        int getMinorNumber(int color) const ;
        int getMajorNumber(int color) const;
        int getMaterialValue(int color) const ;
        int getSide() const; // return the side 
        int getPceNum(int pce) const; // return the number of pieces of the type pce 
        int getPceSq(int pce , int pieceNum) const ; // return which sq the pce at index pieceNum is on
        int getPieceOnSq(int sq) const ; // return t he piece on the square sq 
        int getEnPass() const ; // return the sq of the enpass
        int getFiftyMove() const ; 
        int getHisPly() const;
        int getKingPos(int side) const; 
        U64 getPawns(int color) const { return pawns[color]; }
        
        void setCastlePerm(int newPerm);        
        void setPieceOnSq(int sq , int pce) ; // set the piece on sq to pce 
        void setMaterial(int color , int pce , int d) ; // set the matiral 
        void setBig(int color , int d) ; // set the big Pieces 
        void setMajor(int color, int d) ;
        void setMinor(int color, int d) ;
        void clearBit(int color, int sq) ; 
        void setBit(int color , int sq);
        void setPieceNum(int pce , int d) ; // change the number of pieces of type pce 
        void setPieceList(int pce , int pieceNum, int nSQ) ; // change which sqaure the pce with index pieceNum is on
        void setEnPass(int newEnPass);
        void setFiftyMove(int val);
        void updateFiftyMove(int d);
        void setHisPly(int d);
        void setPly(int d);
        void setKingPos(int side , int sq);
        void ChangeSide();

        int IsAttacked(int sq , int side) const ; // return true if this sq is attacked by the side 
        bool IsKingSideCastl(int side) const; 
        bool IsQueenSideCastl(int side) const;
        void HashPiece(int pce, int sq); // hash the piece 
        void HashCastle();
        void HashSide();
        void HashEnPass();

};