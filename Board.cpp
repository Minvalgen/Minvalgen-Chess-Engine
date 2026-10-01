#include <stdio.h>
#include "Board.hpp"
#include <sstream>
#include <vector>
#include <cctype>


Board::Board(){
    ResetBoard();
    Parse(FEN_STARTUP);
}

void Board::ResetBoard(){
    for(int i = 0 ; i < BOARD_SQ_NUM ; i++){
        pieces[i] = OFF_BOARD;
    }
    for(int i = 0 ; i < 64 ; i++){
        pieces[ConvertToBig(i)] = EMPTY;
    }
    for(int i = 0 ; i < 3 ; i++){
        big[i] = 0;
        major[i] = 0; 
        minor[i] = 0;
        material[i] = 0;
        pawns[i] = 0ULL;
    }
    for(int i = 0 ; i < 13 ; i++){
        pieceNum[i] = 0; 
    }

    kingSq[WHITE] = NO_SQ;
    kingSq[BLACK] = NO_SQ;
    sideToMove = BOTH;
    enPass = NO_SQ;
    fiftyMove = 0;
    ply = 0;
    hisPly = 0;
    castlePerm = 0;
    posKey = 0ULL; 
    
}

int Board::Parse(std::string s) {


    ResetBoard();
    setPos(s);
    auto fen = fenPos.begin();

    int rank = RANK_8;
    int file = FILE_A;

    int count;
    int piece = EMPTY;

    while ((rank >= RANK_1) && *fen && *fen != ' ') {

        count = 1;

        switch (*fen) {

            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;

            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error\n");
                return -1;
        }

        for (int i = 0; i < count; i++) {

            if (file > FILE_H) {
                printf("File error\n");
                return -1;
            }

            int sq = file + 8 * rank;
            int bigSQ = ConvertToBig(sq);

            pieces[bigSQ] = piece;

            file++;
        }

        fen++;
    }

    if (*fen != ' ') {
        printf("FEN format error\n");
        return -1;
    }

    fen++;

    sideToMove = (*fen == 'w') ? WHITE : BLACK;

    fen += 2;

    castlePerm = 0;

    while (*fen != ' ') {

        switch (*fen) {

            case 'K': castlePerm |= WKC; break;
            case 'Q': castlePerm |= WQC; break;
            case 'k': castlePerm |= BKC; break;
            case 'q': castlePerm |= BQC; break;
            case '-': break;

            default:
                printf("Castle error\n");
                return -1;
        }

        fen++;
    }

    fen++;

    enPass = NO_SQ;

    if (*fen != '-') {

        file = fen[0] - 'a';
        rank = fen[1] - '1';

        if (file < FILE_A || file > FILE_H ||
            rank < RANK_1 || rank > RANK_8) {

            printf("En passant error\n");
            return -1;
        }

        enPass = GetSquare(file, rank);
    }

    posKey = GenerateHashKey();
    updateMatrial();
    return 0;
}

U64 Board:: GenerateHashKey(){
    U64 finalKey = 0;
    for(int i = 0 ; i < BOARD_SQ_NUM ; i++){
        int piece = pieces[i];
        if (piece != EMPTY && piece >= wP && piece <= bK){
            finalKey ^= PieceKey[piece][i];
        }
    }
    if (sideToMove == WHITE){
        finalKey ^= SideKey;
    }
    int posEnPass = enPass;
    if (posEnPass != NO_SQ){
        finalKey ^= PieceKey[EMPTY][posEnPass];
    }

    if (castlePerm >= 0 && castlePerm <= 15){
        finalKey ^= CastleKey[castlePerm];
    }

    return finalKey;
}

void Board::PrintBoard(){
    printf("--Game Board--\n\n");
    for(int rank = RANK_8 ; rank >= RANK_1 ; rank--){
        printf("%d ", rank + 1);
        for(int file = FILE_A ; file <= FILE_H ; file++){
            int sq = GetSquare(file , rank);
            int piece = pieces[sq];
            printf("%3c" , PceChar[piece]);
        }
        printf("\n");
    }
    printf("\n");
    printf("  ");
    for(int file = FILE_A ; file <= FILE_H ; file++){
        printf("%3c" , FileChar[file]);
    }
    printf("\n");
    printf("Side to Move :  %3c\n" , SideChar[sideToMove]);
    printf("en pass square : %d\n", enPass);

    printf("Castle : %3c%3c%3c%3c\n" , 
        castlePerm & WQC ? 'Q' : '-',
        castlePerm & WKC ? 'K' : '-', 
        castlePerm & BQC ? 'q' : '-', 
        castlePerm & BKC ? 'k' : '-'
    );

    printf("Position Key :  %llx\n" , posKey);

    std :: cout << "----------------------------\n";
}


void Board::setPos(std::string fen){
    fenPos = fen; 
}

void Board::updateMatrial(){
    for(int i = 0 ; i < BOARD_SQ_NUM ; i++){
        int piece = pieces[i];
        int color = PicColor[piece];
        if ((piece != OFF_BOARD) && (piece != EMPTY)){
            if (PicBig[piece] == TRUE){
                big[color]++;
            }
            if (PicMajor[piece] == TRUE){
                major[color]++;
            }
            if (PicMinor[piece] == TRUE){
                minor[color]++;
            }

            material[color] += PicValue[piece];
            pieceList[piece][pieceNum[piece]] = i; 
            pieceNum[piece]++; 

            if (piece == wK) kingSq[WHITE] = i; 
            if (piece == bK) kingSq[BLACK] = i; 

            int sq = ConvertToSmall(i);
            if (piece == wP){
                SETBIT(pawns[WHITE] , sq);
                SETBIT(pawns[BOTH] , sq);
            }
            if (piece == bP){
                SETBIT(pawns[BLACK] , sq) ;
                SETBIT(pawns[BOTH] , sq);
            }
        }
    }
}

int Board::IsAttacked(int sq, int side) const 
{
    int i, piece, dir, nsq;

    // ======================
    // PAWN ATTACKS (by `side`)
    // ======================
    if (side == WHITE)
    {   

        piece = pieces[sq - 9];
       
        if (piece != OFF_BOARD && piece == wP){
            //  printf("Pawn");
             return TRUE;
        }
            
        piece = pieces[sq - 11];
        if (piece != OFF_BOARD && piece == wP) return TRUE;

    }
    else
    {
        piece = pieces[sq + 9];
        if (piece != OFF_BOARD && piece == bP){
            // printf("Pawn");
             return TRUE;
        }
        piece = pieces[sq + 11];
        if (piece != OFF_BOARD && piece == bP){
            // printf("Pawn1");
            return TRUE;
        }

    }

    // ======================
    // KNIGHTS
    // ======================
    for (i = 0; i < 8; i++)
    {
        nsq = sq + KnDir[i];
        piece = pieces[nsq];

        if (piece != OFF_BOARD &&
            (IsKn(piece) == TRUE) &&
            PicColor[piece] == side)
        {

            return TRUE;
        }
    }

    // ======================
    // ROOK / QUEEN (straight lines)
    // ======================
    for (i = 0; i < 4; i++)
    {
        dir = RkDir[i];
        nsq = sq + dir;

        while (pieces[nsq] != OFF_BOARD)
        {
            piece = pieces[nsq];

            if (piece != EMPTY)
            {
                if ((PicColor[piece] == side) && (IsRQ(piece) == TRUE)){
                    // printf("%d %d\n", PicColor[piece], piece);
                    // printf("RookQ");
                    return TRUE;
                }
                break;
            }

            nsq += dir;
        }
    }

    // ======================
    // BISHOP / QUEEN (diagonals)
    // ======================
    for (i = 0; i < 4; i++)
    {
        dir = BiDir[i];
        nsq = sq + dir;

        while (pieces[nsq] != OFF_BOARD)
        {
            piece = pieces[nsq];

            if (piece != EMPTY)
            {
                if ((PicColor[piece] == side) && (IsBQ(piece) == TRUE)){
                    // printf("BishopQ");
                    return TRUE;
                }
                break;
            }

            nsq += dir;
        }
    }

    // ======================
    // KING
    // ======================
    for (i = 0; i < 8; i++)
    {
        nsq = sq + KiDir[i];
        piece = pieces[nsq];

        if (piece != OFF_BOARD &&
            (IsKi(piece) == TRUE) &&
            PicColor[piece] == side)
        {
            // printf("King");
            return TRUE;
        }
    }

    return FALSE;
}

void Board::PrintAttacked(int side){
      printf("--Game Board--\n\n");
    for(int rank = RANK_8 ; rank >= RANK_1 ; rank--){
        printf("%d ", rank + 1);
        for(int file = FILE_A ; file <= FILE_H ; file++){
            int sq = GetSquare(file , rank);
            if (IsAttacked(sq , side) == TRUE){
                
                printf(" X ");
            }
            else{
                printf(" - ");
            }
        }
        printf("\n");
    }
    printf("\n");
    printf("  ");
    for(int file = FILE_A ; file <= FILE_H ; file++){
        printf("%3c" , FileChar[file]);
    }
}

int Board::getSide() const {
    // printf("i am here too\n");
    return sideToMove;
}
int Board::getPceNum(int pce) const{
    return pieceNum[pce];
} 

int Board::getPceSq(int pce , int pieceNum) const {
    return pieceList[pce][pieceNum];
}

int Board::getPieceOnSq(int sq) const {
    return pieces[sq];
}

void Board::setPieceOnSq(int sq , int pce){
    pieces[sq] = pce; 
}

void Board::setMaterial(int color , int pce , int d){

    material[color] +=  d*PicValue[pce];
}

void Board::setBig(int color , int d){
    big[color] += d;
}

void Board::setMajor(int color , int d){
    major[color] += d; 
}

void Board::setMinor(int color , int d){
    minor[color]+=d; 
}

void Board::clearBit(int color, int sq){
    CLRBIT(pawns[color]  , ConvertToSmall(sq));
}

void Board::setBit(int color , int sq){
    SETBIT(pawns[color] , ConvertToSmall(sq));
}
void Board::setPieceNum(int pce , int d){
    pieceNum[pce] += d; 
} 

void Board::setPieceList(int pce , int pieceNum , int nSQ){
    pieceList[pce][pieceNum] = nSQ;
} 

void Board::HashPiece(int pce, int sq)
{
    posKey ^= PieceKey[pce][sq];
}

void Board::HashCastle()
{
    posKey ^= CastleKey[castlePerm];
}

void Board::HashSide()
{
    posKey ^= SideKey;
}

void Board::HashEnPass()
{
    posKey ^= PieceKey[EMPTY][this->getEnPass()];
}

int Board::getCastlePerm() const{
    return castlePerm;
}

void  Board::setCastlePerm(int newPerm){
    castlePerm = newPerm;
}


int Board::getBigNumber(int color) const {
    return big[color];
}

int Board::getMinorNumber(int color) const{
    return minor[color];
}
int Board::getMajorNumber(int color) const {
    return major[color];
}
int Board::getMaterialValue(int color) const{
    return material[color];
}
int Board :: getEnPass() const {
    return enPass;
}

int Board::getHisPly() const{
    return hisPly;
}

U64 Board::getPosKey() const {
    return posKey;
}

int Board::getFiftyMove() const{
    return fiftyMove;
}

void Board::setEnPass(int newEnPass){
    enPass = newEnPass; 
}

void Board::setFiftyMove(int val){
    fiftyMove = val; 
}

void Board::updateFiftyMove(int d){
    fiftyMove += d; 
}

void Board:: setHisPly(int d){
    hisPly += d; 
}
void Board:: setPly(int d){
    ply += d;
}

void Board:: setKingPos(int side , int sq){
    kingSq[side] = sq; 
}

int Board::getKingPos(int side) const{
    return kingSq[side]; 
}

void Board::ChangeSide(){
    sideToMove ^= 1; 
}
bool Board :: IsKingSideCastl(int side) const{
    if (side == WHITE){
        return ((castlePerm & WKC) != 0);
    } 
    else{
        return ((castlePerm & BKC) != 0);
    }
}

bool Board :: IsQueenSideCastl(int side) const{
    if (side == WHITE){
        return ((castlePerm & WQC) != 0);
    } 
    else{
        return ((castlePerm & BQC) != 0);
    }
}