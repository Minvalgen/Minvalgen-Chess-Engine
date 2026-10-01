#pragma once

#include "Defs.h"
#include "Evaluate.hpp"
#include <vector>
#include <cstdint>

enum TTFlags : uint8_t {
    TT_NONE = 0,
    TT_EXACT = 1,
    TT_ALPHA = 2, // Upper bound
    TT_BETA = 3   // Lower bound
};

struct TTEntry {
    U64 posKey = 0ULL;
    int move = 0;
    int score = 0;
    int depth = 0;
    uint8_t flags = TT_NONE;
    uint8_t age = 0;
};

class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t numEntries = 0;
    uint8_t currentAge = 0;

public:
    TranspositionTable() = default;
    void Init(size_t sizeMB = 16);
    void Clear();
    void NewSearch();

    bool Probe(U64 posKey, int depth, int alpha, int beta, int& bestMove, int& score, int ply);
    void Store(U64 posKey, int move, int score, uint8_t flags, int depth, int ply);
};

extern TranspositionTable TT;
