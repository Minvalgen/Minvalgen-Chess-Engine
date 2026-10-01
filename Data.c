#include "Defs.h"



/* --- for printing ---*/
char PceChar[] = "-PNBRQKpnbrqk";
char SideChar[] = "wb-";
char RankChar[] = "12345678";
char FileChar[] = "abcdefgh";

/* --- for move generator --- */
int LoopSlidePieces[8] = {wB , wR , wQ , 0 , bB , bR , bQ , 0};
int LoopNonSlidePieces[6] = {wN , wK , 0 , bN , bK , 0};
int LoopNonSlideInxed[2] = {0 , 3};
int LoopSlideInxed[2] = {0 , 4};

/* --- for matrial --- */
int PicBig[13] =  {FALSE , FALSE , TRUE, TRUE,TRUE,TRUE,TRUE , FALSE , TRUE, TRUE,TRUE,TRUE,TRUE};
int PicMajor[13] = {FALSE , FALSE , FALSE, FALSE,TRUE,TRUE,TRUE , FALSE , FALSE, FALSE,TRUE,TRUE,TRUE};
int PicMinor[13] = {FALSE , FALSE , TRUE, TRUE, FALSE,FALSE,FALSE , FALSE , TRUE, TRUE,FALSE,FALSE,FALSE};
int PicValue[13] = {0 , 100 , 325, 325, 550, 1000, 500000 , 100 , 325, 325, 550, 1000, 500000};
int PicColor[13] = {BOTH , WHITE , WHITE , WHITE, WHITE , WHITE, WHITE, BLACK, BLACK, BLACK, BLACK,BLACK,BLACK};


int PieceKnight[13] = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE };
int PieceKing[13] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };
int PieceRookQueen[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE };
int PieceBishopQueen[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE };
int PieceSlides[13] = {FALSE , FALSE , FALSE , TRUE , TRUE, TRUE,FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE};
int PiecePawn[13] = { FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };
/* --- for moves ---*/
const int KnDir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10, 1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10, 1, 10, -9, -11, 11, 9 };

const int PceDir[13][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0},
    { 9 , 10, 11, 0, 0, 0, 0, 0},
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -9, -10, -11, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 }
};

const int NumDir[13] = {
    0, 3, 8, 4, 4, 8, 8, 3, 8, 4, 4, 8, 8
};

const int CastleBoard[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};