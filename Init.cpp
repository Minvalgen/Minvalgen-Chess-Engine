#include <stdio.h>
#include "Defs.h"
#include <iostream>

int BigToSmall[BOARD_SQ_NUM];
int SmallToBig[64];
U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKey[13][BOARD_SQ_NUM];
U64 SideKey;
U64 CastleKey[16]; 

int RankBrd[BOARD_SQ_NUM];
int FileBrd[BOARD_SQ_NUM];


int BitTable[64] = {
        63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34,
        61, 29, 2, 51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35,
        62, 31, 40, 4, 49, 5, 52, 26, 60, 6, 23, 44, 46, 27, 56, 16,
        7, 39, 48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36, 8
};

std::string prtSq(int sq){
    int file = FileBrd[sq];
    int rank = RankBrd[sq];
    std::string s = "##";
    s[0] = char('a' + file);
    s[1] = char('1' + rank);
    return s;
}
int PopBits(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

int CountBits(U64 b) {
    int r;
    for (r = 0; b; r++, b &= b - 1);
    return r;
}

void InitBits(){
    for(int i = 0 ; i < 64 ; i++){
        SetMask[i] = (1ULL << i);
        ClearMask[i] = ~SetMask[i];
    }
}

void PrintBitBoard(U64 myBoard)
{
    for (int rank = RANK_8; rank >= RANK_1; rank--) {
        for (int file = FILE_A; file <= FILE_H; file++) {

            int sq = GetSquare(file, rank);
            int bit = BigToSmall[sq];

            if ((myBoard >> bit) & 1)
                printf(" x ");
            else
                printf(" - ");
        }
        printf("\n");
    }
    printf("\n");
}
void Init(){
    for (int i = 0; i < BOARD_SQ_NUM; i++)
        BigToSmall[i] = -1;

    for (int i = 0; i < 64; i++)
        SmallToBig[i] = -1;

    int sq, ind = 0;

    for (int rank = RANK_1; rank <= RANK_8; rank++) {
        for (int file = FILE_A; file <= FILE_H; file++) {
            sq = GetSquare(file, rank);
            SmallToBig[ind] = sq;
            BigToSmall[sq] = ind;
            ind++;
        }
    }
}
void InitHashKey(){
    for(int i = 0 ; i < 13 ; i++){
        for(int j = 0 ; j < BOARD_SQ_NUM ; j++){
            PieceKey[i][j] = RAND();
        }
    }
    SideKey = RAND();
    for(int i = 0 ; i < 16 ; i++) CastleKey[i] = RAND();
}


void InitFileRankBrd(){
    for(int i = 0 ; i < BOARD_SQ_NUM ; i++){
        RankBrd[i] = OFF_BOARD;
        FileBrd[i] = OFF_BOARD;
    }
    for(int rank = RANK_1 ; rank <= RANK_8 ; rank++){
        for(int file = FILE_A ; file <= FILE_H ; file++){
            int sq = GetSquare(file , rank);
            FileBrd[sq] = file;
            RankBrd[sq] = rank; 
        }
    }

    // printf("FileBrd\n");

    // for (int index = 0; index < BOARD_SQ_NUM; index++) {
    //     if (index % 10 == 0 && index != 0) {
    //         printf("\n");
    //     }

    //     printf("%4d", FileBrd[index]);
    // }

    // printf("\n\nRanksBrd\n");

    // for (int index = 0; index < BOARD_SQ_NUM; index++) {
    //     if (index % 10 == 0 && index != 0) {
    //         printf("\n");
    //     }

    //     printf("%4d", RankBrd[index]);
    // }

    // printf("\n");
    
}
void InitAll(){
    Init();
    InitBits();
    InitHashKey();
    InitFileRankBrd();
}