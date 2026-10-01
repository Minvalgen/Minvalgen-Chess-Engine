#pragma once
#include <random>
#include <string>
typedef unsigned long long U64;

#define NAME "Minvalgen 1.0"

#define BOARD_SQ_NUM 120
#define MAX_GAME_MOVES 2048
#define FEN_STARTUP "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAXPOSMOVES 256

enum Boolians{
    FALSE, 
    TRUE
};
enum PieceType {
    EMPTY,
    wP, wN, wB, wR, wQ, wK,
    bP, bN, bB, bR, bQ, bK
};

enum File {
    FILE_A = 0, FILE_B, FILE_C, FILE_D,
    FILE_E, FILE_F, FILE_G, FILE_H,
    FILE_NONE
};

enum Rank {
    RANK_1 = 0, RANK_2, RANK_3, RANK_4,
    RANK_5, RANK_6, RANK_7, RANK_8,
    RANK_NONE
};

enum Color {
    WHITE, BLACK, BOTH
};

enum BoardSquare {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ = 99, OFF_BOARD
};

enum CastlePerm {
    WKC = 1,
    WQC = 2,
    BKC = 4,
    BQC = 8
};


/* Global */
extern int BigToSmall[BOARD_SQ_NUM];
extern int SmallToBig[64];

extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKey[13][BOARD_SQ_NUM];
extern U64 SideKey;
extern U64 CastleKey[16];

extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PicBig[13];
extern int PicMajor[13];
extern int PicMinor[13];
extern int PicValue[13];
extern int PicColor[13];

extern int RankBrd[BOARD_SQ_NUM];
extern int FileBrd[BOARD_SQ_NUM];

extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceSlides[13];
extern int LoopSlidePieces[8];
extern int LoopNonSlidePieces[6];
extern int LoopNonSlideInxed[2];
extern int LoopSlideInxed[2];
extern int PiecePawn[13];
extern const int KnDir[8];
extern const int RkDir[4];
extern const int BiDir[4];
extern const int KiDir[8];
extern const int PceDir[13][8];
extern const int NumDir[13];
extern const int CastleBoard[120]; 

/* Macros */
#define GetSquare(f , r) (21 + f +  10 * r)
#define ConvertToSmall(sq) (BigToSmall[sq])
#define ConvertToBig(sq) (SmallToBig[sq])

#define POP(p) PopBits(p);
#define CNT(p) (CountBits(p))

#define CLRBIT(bb, sq) (bb &= ClearMask[sq])
#define SETBIT(bb, sq) (bb |= SetMask[sq])

#define RAND() ((U64)rand() | (U64)rand() << 15 | (U64)rand() << 30 | (U64)rand() << 45  | ((U64)rand() & 0xf) << 60)

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])
#define IsSlide(p) (PieceSlides[(p)]);

/* Functions */
extern void InitAll();
extern int PopBits(U64 *b);
extern int CountBits(U64 b);
extern void PrintBitBoard(U64 b);
extern std::string prtSq(int sq);

extern int SqOnBoard(int sq);
extern int SideValid(int side);
extern int FileRankValid(int fr);
extern int PieceValid(int piece);
extern int PieceEmptyValid(int piece);