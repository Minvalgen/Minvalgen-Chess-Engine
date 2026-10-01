#pragma once

#include "Defs.h"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "MakeMove.hpp"
#include "Evaluate.hpp"
#include "TranspositionTable.hpp"
#include <chrono>
#include <vector>

#define MAX_PLY 64

struct SearchLimits {
    int maxDepth = 64;
    long long maxTimeMs = 0;  // 0 = unlimited
    long long maxNodes  = 0;  // 0 = unlimited
};

struct SearchResult {
    Move bestMove;
    int  score  = 0;
    int  depth  = 0;
    long long nodes  = 0;
    long long timeMs = 0;
};

class SearchEngine {
private:
    Board*        pos     = nullptr;
    SearchLimits  limits;
    bool          stopped = false;
    long long     nodesCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    // Move-ordering tables
    Move killers[MAX_PLY][2];
    int  history    [13][BOARD_SQ_NUM];  // history heuristic
    int  counterMove[13][BOARD_SQ_NUM];  // countermove heuristic (piece, toSq)

    // Late-move reduction table  [depth][moveIndex]
    int LMRTable[64][64];

    void InitLMRTable();
    void ClearForSearch();
    bool CheckTime();
    bool IsDraw(const Board* pos);

    // Move scoring & helpers
    int  ScoreMove(const Move& move, int ttMove, int ply, int prevMove);
    bool SEEPositive(int fromSq, int toSq) const; // true = capture is at least even

    // Core search
    int Quiescence(int alpha, int beta, int ply);
    int AlphaBeta (int alpha, int beta, int depth, int ply, bool doNull, int prevMove = 0);

public:
    SearchEngine(Board* _pos);
    SearchResult SearchPosition(const SearchLimits& searchLimits);
    void Stop();
    std::vector<Move> GetPVLine(int depth);
};
