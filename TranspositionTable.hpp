#pragma once

#include "Defs.h"
#include <vector>
#include <cstdint>

enum TTFlags : uint8_t {
    TT_NONE  = 0,
    TT_EXACT = 1,
    TT_ALPHA = 2, // Upper bound
    TT_BETA  = 3  // Lower bound
};

#pragma pack(push, 1)
struct TTEntry {
    U64      posKey = 0ULL; // 8 bytes
    int32_t  move   = 0;    // 4 bytes
    int16_t  score  = 0;    // 2 bytes
    int8_t   depth  = 0;    // 1 byte
    uint8_t  genBound = 0;  // 1 byte: upper 6 bits = age, lower 2 bits = bound

    inline uint8_t getFlags() const { return genBound & 3; }
    inline uint8_t getAge()   const { return genBound >> 2; }
    inline void set(uint8_t flags, uint8_t age) {
        genBound = (uint8_t)(((age & 0x3F) << 2) | (flags & 3));
    }
};
#pragma pack(pop)
static_assert(sizeof(TTEntry) == 16, "TTEntry must be exactly 16 bytes");

class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t numEntries = 0;
    size_t mask = 0;
    uint8_t currentAge = 0;

public:
    TranspositionTable() = default;
    void Init(size_t sizeMB = 64);
    void Clear();
    void NewSearch();

    bool Probe(U64 posKey, int depth, int alpha, int beta, int& bestMove, int& score, int ply);
    void Store(U64 posKey, int move, int score, uint8_t flags, int depth, int ply);
};

extern TranspositionTable TT;

