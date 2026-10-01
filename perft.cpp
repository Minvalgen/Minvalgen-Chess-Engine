#include "Perft.hpp"

Perft::Perft(Board *pos){
    this->pos= pos;
}

void Perft::PerfTest(int depth) {
    if (depth == 0){
        leaveNodes++; 
        return; 
    }

    MoveGenerator _gen(pos); 
    _gen.GenerateAllMoves();
    MakeMove engine(pos);
    
    if (leaveNodes - pref >= (int)1e7){
        // std :: cout << leaveNodes << '\n';
        pref = leaveNodes;
    }
    for(int i = 0 ; i < _gen.getCountMoves() ; i++){

        Move move = _gen.getMove(i);
        if (engine.MakeMoves(move) == true){
           PerfTest(depth - 1);
           engine.TakeMove(); 
        }
    }

    return;
}

long long Perft::getNumMoves(){
    return leaveNodes;
}