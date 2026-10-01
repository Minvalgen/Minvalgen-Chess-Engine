#include "TranspositionTable.hpp"

TranspositionTable TT;

void TranspositionTable::Init(size_t sizeMB) {
    size_t bytes = sizeMB * 1024 * 1024;
    numEntries = bytes / sizeof(TTEntry);
    table.assign(numEntries, TTEntry());
    currentAge = 0;
}

void TranspositionTable::Clear() {
    std::fill(table.begin(), table.end(), TTEntry());
}

void TranspositionTable::NewSearch() {
    currentAge++;
}

bool TranspositionTable::Probe(U64 posKey, int depth, int alpha, int beta, int& bestMove, int& score, int ply) {
    if (numEntries == 0) return false;

    size_t index = posKey % numEntries;
    TTEntry& entry = table[index];

    if (entry.posKey == posKey) {
        bestMove = entry.move;

        int retScore = entry.score;
        if (retScore > SCORE_MATE - 1000) retScore -= ply;
        else if (retScore < -SCORE_MATE + 1000) retScore += ply;

        if (entry.depth >= depth) {
            if (entry.flags == TT_EXACT) {
                score = retScore;
                return true;
            }
            if (entry.flags == TT_ALPHA && retScore <= alpha) {
                score = alpha;
                return true;
            }
            if (entry.flags == TT_BETA && retScore >= beta) {
                score = beta;
                return true;
            }
        }
    }
    return false;
}

void TranspositionTable::Store(U64 posKey, int move, int score, uint8_t flags, int depth, int ply) {
    if (numEntries == 0) return;

    size_t index = posKey % numEntries;
    TTEntry& entry = table[index];

    // Replacement strategy: replace if empty, key match, deeper search, or older age
    if (entry.posKey == 0 || entry.posKey == posKey || depth >= entry.depth || entry.age != currentAge) {
        int storeScore = score;
        if (storeScore > SCORE_MATE - 1000) storeScore += ply;
        else if (storeScore < -SCORE_MATE + 1000) storeScore -= ply;

        entry.posKey = posKey;
        entry.move = move;
        entry.score = storeScore;
        entry.depth = depth;
        entry.flags = flags;
        entry.age = currentAge;
    }
}
