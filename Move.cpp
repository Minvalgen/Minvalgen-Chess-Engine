#include "Move.hpp"
#include <iostream>


int Move::FromSq() const{
    return ((move) & (0x7f));
}

int Move::ToSq() const {
    return ((move >> 7) & (0x7f));
}

int Move::Captured() const {
    return ((move >> 14) & (0xf));
}

int Move::Promoted() const {
    return ((move >> 20) & (0xf));
}

bool Move::IsCap() const {
    return (move & (CAP));
}

bool Move::IsCs() const {
    return (move & (CAST));
}

bool Move::IsEp() const {
    return (move & (ENPASS));
}

bool Move::IsPs() const {
    return (move & (PAWNSTART));
}

bool Move::IsPro() const {
    return (move & (PRO));
} 

void Move::setMove(int _move){
    move = _move; 
}

void Move::setScore(int _score){
    score = _score;
}

int Move::getMove() const {
    return move;
}

int Move::getScore() const {
    return score;
}


void Move::PrintMove(){
    for(int i = 30 ; i >= 0 ; i--){
        if ((move  >> i) & 1){
            std::cout << 1;
        }
        else{
            std::cout << 0; 
        }

        if (i % 4 == 0){
            std::cout << " ";
        }
    }
    std :: cout << '\n';
    std::cout <<"----------------------------" << std::endl;
}



void Move::PrtAgeMove(){
    std::cout << GetMoveString();
}

std::string Move::GetMoveString() const {
    if (move == 0) return "0000";
    int fileFrom = FileBrd[FromSq()];
    int rankFrom = RankBrd[FromSq()];
    int fileTo = FileBrd[ToSq()];
    int rankTo = RankBrd[ToSq()];

    std::string s = "";
    s += char('a' + fileFrom);
    s += char('1' + rankFrom);
    s += char('a' + fileTo);
    s += char('1' + rankTo);

    int promoted = Promoted();
    if (promoted) {
        char pchar = 'q';
        if (IsKn(promoted)) pchar = 'n';
        else if (IsRQ(promoted) && !IsBQ(promoted)) pchar = 'r';
        else if (!IsRQ(promoted) && IsBQ(promoted)) pchar = 'b';
        s += pchar;
    }
    return s;
}