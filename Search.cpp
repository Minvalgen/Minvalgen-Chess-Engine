#include "Search.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
static const int SEEValue[13] = {
    0, 100, 325, 335, 500, 975, 20000,
       100, 325, 335, 500, 975, 20000
};

// MVV-LVA: victim value * 10 - attacker value  (higher = better capture)
static const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600,
                                        100, 200, 300, 400, 500, 600 };
static int GetMvvLva(int victim, int attacker) {
    return VictimScore[victim] * 10 + (600 - VictimScore[attacker]);
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
SearchEngine::SearchEngine(Board* _pos) : pos(_pos) {
    TT.Init(64); // 64 MB transposition table
    InitLMRTable();
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void SearchEngine::InitLMRTable() {
    for (int d = 0; d < 64; d++) {
        for (int m = 0; m < 64; m++) {
            if (d == 0 || m == 0) {
                LMRTable[d][m] = 0;
            } else {
                LMRTable[d][m] = std::max(1, (int)(0.77 + std::log((double)d) * std::log((double)m) / 2.36));
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void SearchEngine::ClearForSearch() {
    nodesCount = 0;
    stopped    = false;

    // History gravity: halve history values instead of zeroing them.
    // This keeps useful information while preventing stale bonuses dominating.
    for (int p = 0; p < 13; p++) {
        for (int sq = 0; sq < BOARD_SQ_NUM; sq++) {
            history[p][sq] /= 2;
        }
    }

    std::memset(killers,      0, sizeof(killers));
    std::memset(counterMove,  0, sizeof(counterMove));
    TT.NewSearch();
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void SearchEngine::Stop() { stopped = true; }

bool SearchEngine::CheckTime() {
    if (limits.maxTimeMs > 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        if (ms >= limits.maxTimeMs) { stopped = true; return true; }
    }
    if (limits.maxNodes > 0 && nodesCount >= limits.maxNodes) {
        stopped = true; return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
bool SearchEngine::IsDraw(const Board* p) {
    if (p->getFiftyMove() >= 100) return true;

    // 3-fold repetition
    int hisPly   = p->getHisPly();
    int fiftyMove = p->getFiftyMove();
    U64 key      = p->getPosKey();
    int start    = std::max(0, hisPly - fiftyMove);
    int reps     = 0;
    for (int i = start; i < hisPly; i++) {
        if (p->history[i].getPosKey() == key) {
            if (++reps >= 2) return true; // 3rd occurrence = draw
        }
    }

    // Insufficient material
    if (p->getPceNum(wP) == 0 && p->getPceNum(bP) == 0) {
        // KK, KNK, KBK, KNKN, KBKB all insufficient
        if (p->getMaterialValue(WHITE) <= 335 && p->getMaterialValue(BLACK) <= 335)
            return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
bool SearchEngine::SEEPositive(int fromSq, int toSq) const {
    int fromPce = pos->getPieceOnSq(fromSq);
    int toPce   = pos->getPieceOnSq(toSq);
    if (toPce == EMPTY) return true; // e.p. or promotion – treat as OK

    // If we capture a more valuable piece, always good.
    if (SEEValue[toPce] >= SEEValue[fromPce]) return true;

    // If we capture a less valuable piece, check if it is defended.
    int oppSide = pos->getSide() ^ 1; // opponent's side (they defend toSq)
    if (pos->IsAttacked(toSq, oppSide)) {
        // They can recapture.  Is the exchange still winning?
        // Gain = value[toPce] - value[fromPce]  (we'd lose our piece)
        return (SEEValue[toPce] - SEEValue[fromPce]) >= 0;
    }
    // Square is undefended – free capture always good
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::ScoreMove(const Move& move, int ttMove, int ply, int prevMove) {
    // 1) TT move
    if (ttMove != 0 && move.getMove() == ttMove) return 2000000;

    // 2) Winning/even captures (MVV-LVA, SEE filtered)
    int cap = move.Captured();
    if (cap != EMPTY || move.IsEp()) {
        int fromPce = pos->getPieceOnSq(move.FromSq());
        int victim  = move.IsEp() ? (pos->getSide() == WHITE ? bP : wP) : cap;
        int mvvlva  = GetMvvLva(victim, fromPce);
        // Good captures above quiet moves, bad captures below
        if (SEEPositive(move.FromSq(), move.ToSq())) {
            return 1000000 + mvvlva;
        } else {
            return -1000000 + mvvlva; // losing capture – search last
        }
    }

    // 3) Promotions
    int pro = move.Promoted();
    if (pro != EMPTY) return 950000 + SEEValue[pro];

    // 4) Killers
    if (ply < MAX_PLY) {
        if (killers[ply][0].getMove() != 0 && move.getMove() == killers[ply][0].getMove()) return 900000;
        if (killers[ply][1].getMove() != 0 && move.getMove() == killers[ply][1].getMove()) return 800000;
    }

    // 5) Countermove bonus
    if (prevMove != 0) {
        Move pm; pm.setMove(prevMove);
        int pmPce = pos->getPieceOnSq(pm.FromSq()); // may be EMPTY after make/unmake but safe
        if (counterMove[pmPce][pm.ToSq()] == move.getMove()) return 700000;
    }

    // 6) History score
    int pce = pos->getPieceOnSq(move.FromSq());
    return history[pce][move.ToSq()];
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::Quiescence(int alpha, int beta, int ply) {
    nodesCount++;
    if ((nodesCount & 4095) == 0 && CheckTime()) return 0;

    int standPat = Evaluation::Evaluate(pos);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    // Delta pruning: if even capturing the queen can't raise alpha, give up
    if (standPat + 1100 < alpha) return alpha;

    MoveGenerator gen(pos);
    gen.GenerateCaptures();

    MakeMove engine(pos);
    int n = gen.getCountMoves();

    // Score + sort captures
    std::vector<std::pair<int, Move>> scoredMoves;
    scoredMoves.reserve(n);
    for (int i = 0; i < n; i++) {
        Move m = gen.getMove(i);
        scoredMoves.push_back({ ScoreMove(m, 0, ply, 0), m });
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b){ return a.first > b.first; });

    for (const auto& pair : scoredMoves) {
        Move move = pair.second;

        // SEE pruning: skip clearly losing captures in QSearch
        int cap = move.Captured();
        if (cap != EMPTY && move.Promoted() == EMPTY && !move.IsEp()) {
            if (!SEEPositive(move.FromSq(), move.ToSq())) continue;
        }

        if (!engine.MakeMoves(move)) continue;

        int score = -Quiescence(-beta, -alpha, ply + 1);
        engine.TakeMove();

        if (stopped) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::AlphaBeta(int alpha, int beta, int depth, int ply, bool doNull, int prevMove) {
    nodesCount++;
    if ((nodesCount & 4095) == 0 && CheckTime()) return 0;

    // Draw check (not at root)
    if (ply > 0 && IsDraw(pos)) return 0;

    // Check extension – must happen BEFORE depth-zero drop
    bool inCheck = pos->IsAttacked(pos->getKingPos(pos->getSide()), pos->getSide() ^ 1);
    if (inCheck) depth++;

    // Drop into QSearch
    if (depth <= 0) return Quiescence(alpha, beta, ply);

    // Ply limit guard
    if (ply >= MAX_PLY - 1) return Evaluation::Evaluate(pos);

    // Mate distance pruning
    int mateScore = SCORE_MATE - ply;
    if (alpha < -mateScore) alpha = -mateScore;
    if (beta  >  mateScore) beta  =  mateScore;
    if (alpha >= beta) return alpha;

    // Transposition table probe
    int ttMove  = 0;
    int ttScore = 0;
    if (TT.Probe(pos->getPosKey(), depth, alpha, beta, ttMove, ttScore, ply)) {
        return ttScore;
    }

    int staticEval = Evaluation::Evaluate(pos);

    // ── Razoring (depth 1 only) ───────────────────────────────────────────
    if (!inCheck && depth == 1 && staticEval + 300 < alpha) {
        return Quiescence(alpha, beta, ply);
    }

    // ── Reverse Futility Pruning (Static Null Move) ───────────────────────
    if (!inCheck && depth <= 3 && std::abs(beta) < SCORE_MATE - 1000) {
        int rfpMargin = 90 * depth;
        if (staticEval - rfpMargin >= beta) return staticEval;
    }

    // ── Null Move Pruning ─────────────────────────────────────────────────
    MakeMove engine(pos);
    if (doNull && !inCheck && depth >= 3 && pos->getBigNumber(pos->getSide()) > 0
        && staticEval >= beta) {
        engine.MakeNullMove();
        int R = 2 + depth / 4 + std::min(3, (staticEval - beta) / 150);
        R = std::min(R, depth - 1);
        int nullScore = -AlphaBeta(-beta, -beta + 1, depth - 1 - R, ply + 1, false);
        engine.TakeNullMove();
        if (stopped) return 0;
        if (nullScore >= beta && std::abs(nullScore) < SCORE_MATE - 1000) return beta;
    }

    // ── Move Generation & Sorting ─────────────────────────────────────────
    MoveGenerator gen(pos);
    gen.GenerateAllMoves();

    int n = gen.getCountMoves();
    std::vector<std::pair<int, Move>> scoredMoves;
    scoredMoves.reserve(n);
    for (int i = 0; i < n; i++) {
        Move m = gen.getMove(i);
        scoredMoves.push_back({ ScoreMove(m, ttMove, ply, prevMove), m });
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b){ return a.first > b.first; });

    int  legalMoves = 0;
    int  bestScore  = -SCORE_INFINITY;
    Move bestMove;
    uint8_t ttFlag  = TT_ALPHA;

    for (int idx = 0; idx < (int)scoredMoves.size(); idx++) {
        Move move = scoredMoves[idx].second;

        if (!engine.MakeMoves(move)) continue;
        legalMoves++;

        bool isQuiet = (move.Captured() == EMPTY && move.Promoted() == EMPTY && !move.IsEp());

        // ── Futility Pruning (quiet moves only) ───────────────────────────
        if (isQuiet && !inCheck && depth <= 3 && legalMoves > 1 && std::abs(alpha) < SCORE_MATE - 1000) {
            static const int FutilityMargin[4] = { 0, 100, 300, 500 };
            if (staticEval + FutilityMargin[depth] <= alpha) {
                engine.TakeMove();
                continue;
            }
        }

        int score = 0;

        if (legalMoves == 1) {
            // First move: full-window search
            score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1, true, move.getMove());
        } else {
            // ── Late Move Reductions ──────────────────────────────────────
            int reduction = 0;
            if (depth >= 3 && isQuiet && !inCheck && legalMoves > 3) {
                reduction = LMRTable[std::min(depth, 63)][std::min(legalMoves, 63)];
                // Extra reduction for late quiet moves
                if (legalMoves > 8) reduction++;
                // Don't reduce killer moves
                if (ply < MAX_PLY &&
                    (move.getMove() == killers[ply][0].getMove() ||
                     move.getMove() == killers[ply][1].getMove())) {
                    reduction = std::max(0, reduction - 1);
                }
                reduction = std::min(reduction, depth - 2);
            }

            // Null-window search with possible reduction
            score = -AlphaBeta(-alpha - 1, -alpha, depth - 1 - reduction, ply + 1, true, move.getMove());

            // Re-search at full depth if reduced search raised alpha
            if (score > alpha && reduction > 0) {
                score = -AlphaBeta(-alpha - 1, -alpha, depth - 1, ply + 1, true, move.getMove());
            }
            // Re-search with full window if PVS null-window raises alpha
            if (score > alpha && score < beta) {
                score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1, true, move.getMove());
            }
        }

        engine.TakeMove();
        if (stopped) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;
        }

        if (score >= beta) {
            // Beta cutoff
            if (isQuiet && ply < MAX_PLY) {
                // Update killers
                killers[ply][1] = killers[ply][0];
                killers[ply][0] = move;
                // Update history (capped to avoid overflow)
                int pce = pos->getPieceOnSq(move.FromSq());
                history[pce][move.ToSq()] = std::min(history[pce][move.ToSq()] + depth * depth, 30000);
                // Update countermove
                if (prevMove != 0) {
                    Move pm; pm.setMove(prevMove);
                    int pmPce = pos->getPieceOnSq(pm.FromSq());
                    counterMove[pmPce][pm.ToSq()] = move.getMove();
                }
            }
            TT.Store(pos->getPosKey(), move.getMove(), beta, TT_BETA, depth, ply);
            return beta;
        }

        if (score > alpha) {
            alpha  = score;
            ttFlag = TT_EXACT;
        }
    }

    // Terminal node
    if (legalMoves == 0) {
        return inCheck ? (-SCORE_MATE + ply) : 0;
    }

    TT.Store(pos->getPosKey(), bestMove.getMove(), bestScore, ttFlag, depth, ply);
    return bestScore;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
SearchResult SearchEngine::SearchPosition(const SearchLimits& searchLimits) {
    limits    = searchLimits;
    startTime = std::chrono::high_resolution_clock::now();
    ClearForSearch();

    SearchResult result;
    int prevScore = 0;

    for (int depth = 1; depth <= limits.maxDepth; depth++) {
        int score;

        if (depth <= 4) {
            // No aspiration window for shallow depths
            score = AlphaBeta(-SCORE_INFINITY, SCORE_INFINITY, depth, 0, true);
        } else {
            // Aspiration window: start narrow, widen on fail
            int delta = 25;
            int aspirAlpha = prevScore - delta;
            int aspirBeta  = prevScore + delta;

            while (true) {
                score = AlphaBeta(aspirAlpha, aspirBeta, depth, 0, true);

                if (stopped) break;

                if (score <= aspirAlpha) {
                    // Fail low: widen lower bound
                    aspirAlpha = std::max(-SCORE_INFINITY, aspirAlpha - delta);
                    delta      = std::min(delta * 3, 500);
                } else if (score >= aspirBeta) {
                    // Fail high: widen upper bound
                    aspirBeta  = std::min(SCORE_INFINITY, aspirBeta + delta);
                    delta      = std::min(delta * 3, 500);
                } else {
                    break; // Exact score within window
                }
            }
        }

        if (stopped && depth > 1) break;
        prevScore = score;

        // Extract best move from TT
        int ttMove = 0, dummy = 0;
        TT.Probe(pos->getPosKey(), depth, -SCORE_INFINITY, SCORE_INFINITY, ttMove, dummy, 0);
        if (ttMove != 0) result.bestMove.setMove(ttMove);
        result.score = score;
        result.depth = depth;
        result.nodes = nodesCount;

        auto now       = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        result.timeMs  = durationMs;
        long long nps  = durationMs > 0 ? (nodesCount * 1000 / durationMs) : 0;

        // UCI info output
        std::cout << "info depth " << depth << " score ";
        if      (score >  SCORE_MATE - 1000) std::cout << "mate " << ( SCORE_MATE - score + 1) / 2;
        else if (score < -SCORE_MATE + 1000) std::cout << "mate " << (-SCORE_MATE - score) / 2;
        else                                  std::cout << "cp " << score;
        std::cout << " nodes " << nodesCount << " nps " << nps << " time " << durationMs << " pv ";

        std::vector<Move> pv = GetPVLine(depth);
        for (const Move& m : pv) std::cout << m.GetMoveString() << " ";
        std::cout << std::endl;

        // Soft time limit: stop if we've used more than 60% of allocated time
        // (the remaining 40% is the hard limit enforced by CheckTime)
        if (limits.maxTimeMs > 0 && durationMs * 10 >= limits.maxTimeMs * 6) break;
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
std::vector<Move> SearchEngine::GetPVLine(int maxDepth) {
    std::vector<Move> pv;
    MakeMove engine(pos);
    std::vector<U64> seen;

    for (int d = 0; d < maxDepth; d++) {
        U64 key = pos->getPosKey();
        if (std::find(seen.begin(), seen.end(), key) != seen.end()) break;
        seen.push_back(key);

        int moveVal = 0, sc = 0;
        if (!TT.Probe(key, 0, -SCORE_INFINITY, SCORE_INFINITY, moveVal, sc, d) || moveVal == 0) break;

        Move m; m.setMove(moveVal);
        if (!engine.MakeMoves(m)) break;
        pv.push_back(m);
    }
    for (size_t i = 0; i < pv.size(); i++) engine.TakeMove();
    return pv;
}
