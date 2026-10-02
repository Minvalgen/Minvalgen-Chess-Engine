#include "TranspositionTable.hpp"
#include "Evaluate.hpp"
#include <algorithm>

TranspositionTable TT;

void TranspositionTable::Init(size_t sizeMB) {
    size_t bytes = sizeMB * 1024 * 1024;
    size_t entries = 1;
    while ((entries * 2 * sizeof(TTEntry)) <= bytes) {
        entries *= 2;
    }
    // If already allocated with this exact capacity, do not reallocate or wipe!
    if (numEntries == entries && table.size() == entries) {
        return;
    }
    numEntries = entries;
    mask = numEntries - 1;
    table.assign(numEntries, TTEntry{});
    currentAge = 0;
}

void TranspositionTable::Clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
    currentAge = 0;
}

void TranspositionTable::NewSearch() {
    currentAge = (currentAge + 1) & 0x3F;
}

bool TranspositionTable::Probe(U64 posKey, int depth, int alpha, int beta, int& bestMove, int& score, int ply) {
    if (numEntries == 0) return false;

    size_t index = posKey & mask;
    const TTEntry& entry = table[index];

    if (entry.posKey == posKey) {
        bestMove = entry.move;

        int retScore = entry.score;
        if (retScore > SCORE_MATE - 1000) retScore -= ply;
        else if (retScore < -SCORE_MATE + 1000) retScore += ply;

        if (entry.depth >= depth) {
            uint8_t flags = entry.getFlags();
            if (flags == TT_EXACT) {
                score = retScore;
                return true;
            }
            if (flags == TT_ALPHA && retScore <= alpha) {
                score = retScore;
                return true;
            }
            if (flags == TT_BETA && retScore >= beta) {
                score = retScore;
                return true;
            }
        }
    }
    return false;
}

void TranspositionTable::Store(U64 posKey, int move, int score, uint8_t flags, int depth, int ply) {
    if (numEntries == 0) return;

    size_t index = posKey & mask;
    TTEntry& entry = table[index];

    bool replace = false;
    if (entry.posKey == 0) {
        replace = true;
    } else if (entry.posKey == posKey) {
        // Same position: replace if search is at least as deep, or if exact score
        if (depth >= entry.depth - 2 || flags == TT_EXACT) {
            replace = true;
        }
    } else {
        // Different position: replace if from older search or if new search is deeper
        if (entry.getAge() != currentAge || depth >= entry.depth) {
            replace = true;
        }
    }

    if (replace) {
        int storeScore = score;
        if (storeScore > SCORE_MATE - 1000) storeScore += ply;
        else if (storeScore < -SCORE_MATE + 1000) storeScore -= ply;

        entry.posKey = posKey;
        if (move != 0 || entry.posKey != posKey) {
            entry.move = move;
        }
        entry.score = (int16_t)storeScore;
        entry.depth = (int8_t)depth;
        entry.set(flags, currentAge);
    } else if (entry.posKey == posKey && move != 0 && entry.move == 0) {
        entry.move = move;
    }
}

