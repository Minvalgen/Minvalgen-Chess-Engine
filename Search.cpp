#include "Search.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Piece values for Static Exchange Evaluation & Move Ordering
// ─────────────────────────────────────────────────────────────────────────────
static const int SEEValue[13] = {
    0, 100, 325, 335, 500, 975, 20000,
       100, 325, 335, 500, 975, 20000
};

// MVV-LVA: victim value * 10 - attacker value (higher = better capture)
static const int VictimScore[13] = {
    0, 100, 200, 300, 400, 500, 600,
       100, 200, 300, 400, 500, 600
};

static inline int GetMvvLva(int victim, int attacker) {
    return VictimScore[victim] * 10 + (600 - VictimScore[attacker]);
}

// ─────────────────────────────────────────────────────────────────────────────
// SearchEngine Constructor & Initialization
// ─────────────────────────────────────────────────────────────────────────────
SearchEngine::SearchEngine(Board* _pos) : pos(_pos), maker(_pos) {
    TT.Init(64); // 64 MB transposition table (persistent, will not reallocate if already 64MB)
    InitLMRTable();
}

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

void SearchEngine::ClearForSearch() {
    nodesCount = 0;
    stopped    = false;

    // History gravity: halve history values instead of zeroing them.
    for (int p = 0; p < 13; p++) {
        for (int sq = 0; sq < BOARD_SQ_NUM; sq++) {
            history[p][sq] /= 2;
        }
    }

    std::memset(killers,      0, sizeof(killers));
    std::memset(counterMove,  0, sizeof(counterMove));
    std::memset(pvTable,      0, sizeof(pvTable));
    std::memset(pvLength,     0, sizeof(pvLength));
    TT.NewSearch();
}

void SearchEngine::Stop() {
    stopped = true;
}

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

bool SearchEngine::IsDraw(const Board* p) {
    if (p->getFiftyMove() >= 100) return true;

    // 3-fold repetition
    int hisPly    = p->getHisPly();
    int fiftyMove = p->getFiftyMove();
    U64 key       = p->getPosKey();
    int start     = std::max(0, hisPly - fiftyMove);
    int reps      = 0;
    for (int i = start; i < hisPly; i++) {
        if (p->history[i].getPosKey() == key) {
            if (++reps >= 2) return true; // 3rd occurrence = draw
        }
    }

    // Insufficient material
    if (p->getPceNum(wP) == 0 && p->getPceNum(bP) == 0) {
        if (p->getMaterialValue(WHITE) <= 335 && p->getMaterialValue(BLACK) <= 335)
            return true;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Move Scoring
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::ScoreMove(const Move& move, int ttMove, int ply, int prevMove) {
    int m = move.getMove();

    // 1) TT move - searched first
    if (ttMove != 0 && m == ttMove) return 2000000;

    // 2) Captures & En-passant
    int cap = move.Captured();
    if (cap != EMPTY || move.IsEp()) {
        int fromPce = pos->getPieceOnSq(move.FromSq());
        int victim  = move.IsEp() ? (pos->getSide() == WHITE ? bP : wP) : cap;
        int mvvlva  = GetMvvLva(victim, fromPce);

        // Winning or equal captures go before quiet moves
        if (SEEValue[victim] >= SEEValue[fromPce]) {
            return 1000000 + mvvlva;
        } else {
            // Under-capture: check if defended by opponent
            if (!pos->IsAttacked(move.ToSq(), pos->getSide() ^ 1)) {
                return 1000000 + mvvlva;
            }
            return -1000000 + mvvlva; // Losing capture – search after quiets
        }
    }

    // 3) Promotions
    int pro = move.Promoted();
    if (pro != EMPTY) {
        if (pro == wQ || pro == bQ) return 950000;
        return 600000; // Under-promotions
    }

    // 4) Killers
    if (ply < MAX_PLY) {
        if (killers[ply][0].getMove() != 0 && m == killers[ply][0].getMove()) return 900000;
        if (killers[ply][1].getMove() != 0 && m == killers[ply][1].getMove()) return 800000;
    }

    // 5) Countermove bonus
    if (prevMove != 0) {
        Move pm; pm.setMove(prevMove);
        int pmPce = pos->getPieceOnSq(pm.ToSq());
        if (pmPce != EMPTY && counterMove[pmPce][pm.ToSq()] == m) {
            return 700000;
        }
    }

    // 6) History score
    int pce = pos->getPieceOnSq(move.FromSq());
    return history[pce][move.ToSq()];
}

// ─────────────────────────────────────────────────────────────────────────────
// Quiescence Search (QSearch)
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::Quiescence(int alpha, int beta, int ply) {
    nodesCount++;
    if ((nodesCount & 4095) == 0 && CheckTime()) return 0;

    bool inCheck = pos->IsAttacked(pos->getKingPos(pos->getSide()), pos->getSide() ^ 1);

    if (!inCheck) {
        int standPat = Evaluation::Evaluate(pos);
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;

        // Delta pruning: if even capturing queen can't raise alpha, give up
        if (standPat + 1100 < alpha) return alpha;
    }

    // Ply limit guard
    if (ply >= MAX_PLY - 1) return Evaluation::Evaluate(pos);

    MoveGenerator gen(pos);
    if (inCheck) {
        gen.GenerateAllMoves(); // Must escape check
    } else {
        gen.GenerateCaptures(); // Direct captures and promotions only
    }

    int countMoves = gen.getCountMoves();
    Move moves[MAXPOSMOVES];
    int scores[MAXPOSMOVES];

    for (int i = 0; i < countMoves; i++) {
        moves[i] = gen.getMove(i);
        int cap = moves[i].Captured();
        if (cap != EMPTY || moves[i].IsEp()) {
            int fromPce = pos->getPieceOnSq(moves[i].FromSq());
            int victim  = moves[i].IsEp() ? (pos->getSide() == WHITE ? bP : wP) : cap;
            scores[i] = GetMvvLva(victim, fromPce);
        } else if (moves[i].Promoted() != EMPTY) {
            scores[i] = 1000 + SEEValue[moves[i].Promoted()];
        } else {
            scores[i] = 0; // Quiet check evasion
        }
    }

    int legalMoves = 0;

    for (int i = 0; i < countMoves; i++) {
        // Selection sort: find max score in i..countMoves-1
        int bestIdx = i;
        for (int j = i + 1; j < countMoves; j++) {
            if (scores[j] > scores[bestIdx]) bestIdx = j;
        }
        std::swap(moves[i], moves[bestIdx]);
        std::swap(scores[i], scores[bestIdx]);

        Move move = moves[i];

        // SEE pruning: skip clearly losing captures in QSearch when not in check
        if (!inCheck && move.Captured() != EMPTY && move.Promoted() == EMPTY && !move.IsEp()) {
            int fromPce = pos->getPieceOnSq(move.FromSq());
            int toPce   = pos->getPieceOnSq(move.ToSq());
            if (SEEValue[toPce] < SEEValue[fromPce]) {
                if (pos->IsAttacked(move.ToSq(), pos->getSide() ^ 1)) {
                    continue;
                }
            }
        }

        if (!maker.MakeMoves(move)) continue;
        legalMoves++;

        int score = -Quiescence(-beta, -alpha, ply + 1);
        maker.TakeMove();

        if (stopped) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    // Checkmate detection in QSearch if in check
    if (inCheck && legalMoves == 0) {
        return -SCORE_MATE + ply;
    }

    return alpha;
}

// ─────────────────────────────────────────────────────────────────────────────
// Alpha-Beta Search (Negamax with PVS)
// ─────────────────────────────────────────────────────────────────────────────
int SearchEngine::AlphaBeta(int alpha, int beta, int depth, int ply, bool doNull, int prevMove) {
    nodesCount++;
    if ((nodesCount & 4095) == 0 && CheckTime()) return 0;

    pvLength[ply] = 0;

    // Draw check (not at root)
    if (ply > 0 && IsDraw(pos)) return 0;

    // Check extension – must happen BEFORE depth-zero drop
    bool inCheck = pos->IsAttacked(pos->getKingPos(pos->getSide()), pos->getSide() ^ 1);
    if (inCheck && ply < MAX_PLY - 1) depth++;

    // Drop into QSearch
    if (depth <= 0) return Quiescence(alpha, beta, ply);

    // Ply limit guard
    if (ply >= MAX_PLY - 1) return Evaluation::Evaluate(pos);

    // Mate distance pruning
    int mateScore = SCORE_MATE - ply;
    if (alpha < -mateScore) alpha = -mateScore;
    if (beta  >  mateScore) beta  =  mateScore;
    if (alpha >= beta) return alpha;

    // Transposition table probe (do NOT cutoff at root ply == 0)
    int ttMove  = 0;
    int ttScore = 0;
    if (TT.Probe(pos->getPosKey(), depth, alpha, beta, ttMove, ttScore, ply)) {
        if (ply > 0) return ttScore;
    }

    int staticEval = Evaluation::Evaluate(pos);

    // ── Razoring (depth 1 only) ───────────────────────────────────────────
    if (!inCheck && depth == 1 && staticEval + 300 < alpha) {
        return Quiescence(alpha, beta, ply);
    }

    // ── Reverse Futility Pruning (Static Null Move) ───────────────────────
    if (!inCheck && depth <= 6 && std::abs(beta) < SCORE_MATE - 1000) {
        int rfpMargin = 75 * depth;
        if (staticEval - rfpMargin >= beta) return beta;
    }

    // ── Null Move Pruning ─────────────────────────────────────────────────
    if (doNull && !inCheck && depth >= 3 && pos->getBigNumber(pos->getSide()) > 0
        && staticEval >= beta) {
        maker.MakeNullMove();
        int R = 2 + depth / 4 + std::min(3, (staticEval - beta) / 150);
        R = std::min(R, depth - 1);
        int nullScore = -AlphaBeta(-beta, -beta + 1, depth - 1 - R, ply + 1, false);
        maker.TakeNullMove();
        if (stopped) return 0;
        if (nullScore >= beta && std::abs(nullScore) < SCORE_MATE - 1000) return beta;
    }

    // ── Move Generation & Scoring (Fixed Stack Arrays) ────────────────────
    MoveGenerator gen(pos);
    gen.GenerateAllMoves();

    int countMoves = gen.getCountMoves();
    Move moves[MAXPOSMOVES];
    int scores[MAXPOSMOVES];

    for (int i = 0; i < countMoves; i++) {
        moves[i] = gen.getMove(i);
        scores[i] = ScoreMove(moves[i], ttMove, ply, prevMove);
    }

    int  legalMoves = 0;
    int  bestScore  = -SCORE_INFINITY;
    Move bestMove;
    uint8_t ttFlag  = TT_ALPHA;

    Move quietMovesTried[MAXPOSMOVES];
    int numQuietsTried = 0;

    for (int idx = 0; idx < countMoves; idx++) {
        // Selection sort: find max score in idx..countMoves-1
        int bestIdx = idx;
        for (int j = idx + 1; j < countMoves; j++) {
            if (scores[j] > scores[bestIdx]) bestIdx = j;
        }
        std::swap(moves[idx], moves[bestIdx]);
        std::swap(scores[idx], scores[bestIdx]);

        Move move = moves[idx];
        bool isQuiet = (move.Captured() == EMPTY && move.Promoted() == EMPTY && !move.IsEp());

        // ── Late Move Pruning (LMP) ──────────────────────────────────────
        if (isQuiet && !inCheck && depth <= 4 && legalMoves >= (3 + 3 * depth * depth)) {
            continue;
        }

        // ── Futility Pruning (quiet moves only) ───────────────────────────
        if (isQuiet && !inCheck && depth <= 3 && legalMoves > 0 && std::abs(alpha) < SCORE_MATE - 1000) {
            static const int FutilityMargin[4] = { 0, 100, 300, 500 };
            if (staticEval + FutilityMargin[depth] <= alpha) {
                continue;
            }
        }

        if (!maker.MakeMoves(move)) continue;
        legalMoves++;

        if (isQuiet) {
            quietMovesTried[numQuietsTried++] = move;
        }

        int score = 0;

        if (legalMoves == 1) {
            // First move: full-window search
            score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1, true, move.getMove());
        } else {
            // ── Late Move Reductions (LMR) ──────────────────────────────
            int reduction = 0;
            bool givesCheck = pos->IsAttacked(pos->getKingPos(pos->getSide()), pos->getSide() ^ 1);

            if (depth >= 3 && isQuiet && !inCheck && !givesCheck && legalMoves > 3) {
                reduction = LMRTable[std::min(depth, 63)][std::min(legalMoves, 63)];
                if (legalMoves > 8) reduction++;
                if (ply < MAX_PLY &&
                    (move.getMove() == killers[ply][0].getMove() ||
                     move.getMove() == killers[ply][1].getMove())) {
                    reduction = std::max(0, reduction - 1);
                }
                if (beta > alpha + 1) {
                    reduction = std::max(0, reduction - 1); // Reduce less at PV nodes
                }
                int pce = pos->getPieceOnSq(move.ToSq());
                if (pce != EMPTY) {
                    if (history[pce][move.ToSq()] > 10000) reduction--;
                    else if (history[pce][move.ToSq()] < -5000) reduction++;
                }
                reduction = std::max(0, std::min(reduction, depth - 2));
            }

            // Null-window search with possible reduction
            score = -AlphaBeta(-alpha - 1, -alpha, depth - 1 - reduction, ply + 1, true, move.getMove());

            // Re-search at full depth if reduced search raised alpha
            if (score > alpha && reduction > 0) {
                score = -AlphaBeta(-alpha - 1, -alpha, depth - 1, ply + 1, true, move.getMove());
            }
            // Re-search with full window if PVS null-window raises alpha at PV node
            if (score > alpha && score < beta) {
                score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1, true, move.getMove());
            }
        }

        maker.TakeMove();
        if (stopped) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;
        }

        if (score >= beta) {
            // Beta cutoff
            if (isQuiet && ply < MAX_PLY) {
                // Update killers
                if (killers[ply][0].getMove() != move.getMove()) {
                    killers[ply][1] = killers[ply][0];
                    killers[ply][0] = move;
                }
                // History bonus & malus
                int bonus = depth * depth;
                int pce = pos->getPieceOnSq(move.FromSq());
                history[pce][move.ToSq()] = std::min(history[pce][move.ToSq()] + bonus, 30000);

                for (int q = 0; q < numQuietsTried - 1; q++) {
                    int qPce = pos->getPieceOnSq(quietMovesTried[q].FromSq());
                    history[qPce][quietMovesTried[q].ToSq()] = std::max(history[qPce][quietMovesTried[q].ToSq()] - bonus, -30000);
                }

                // Update countermove
                if (prevMove != 0) {
                    Move pm; pm.setMove(prevMove);
                    int pmPce = pos->getPieceOnSq(pm.ToSq());
                    if (pmPce != EMPTY) counterMove[pmPce][pm.ToSq()] = move.getMove();
                }
            }
            TT.Store(pos->getPosKey(), move.getMove(), score, TT_BETA, depth, ply);
            return beta;
        }

        if (score > alpha) {
            alpha  = score;
            ttFlag = TT_EXACT;

            // Update PV Table
            pvTable[ply][ply] = move;
            for (int next = 0; next < pvLength[ply + 1]; next++) {
                pvTable[ply][ply + 1 + next] = pvTable[ply + 1][ply + 1 + next];
            }
            pvLength[ply] = 1 + pvLength[ply + 1];
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
// Iterative Deepening Search
// ─────────────────────────────────────────────────────────────────────────────
SearchResult SearchEngine::SearchPosition(const SearchLimits& searchLimits) {
    limits    = searchLimits;
    startTime = std::chrono::high_resolution_clock::now();
    ClearForSearch();

    SearchResult result;
    int prevScore = 0;

    for (int depth = 1; depth <= limits.maxDepth; depth++) {
        int score = 0;

        if (depth <= 4) {
            // No aspiration window for shallow depths
            score = AlphaBeta(-SCORE_INFINITY, SCORE_INFINITY, depth, 0, true);
        } else {
            // Aspiration window: start narrow, widen on fail
            int delta = 25;
            int aspirAlpha = prevScore - delta;
            int aspirBeta  = prevScore + delta;
            int failedCount = 0;

            while (true) {
                score = AlphaBeta(aspirAlpha, aspirBeta, depth, 0, true);

                if (stopped) break;

                if (score <= aspirAlpha) {
                    failedCount++;
                    if (failedCount >= 3 || delta >= 400) {
                        aspirAlpha = -SCORE_INFINITY;
                    } else {
                        delta = std::min(delta * 2, 400);
                        aspirAlpha = std::max(-SCORE_INFINITY, prevScore - delta);
                    }
                } else if (score >= aspirBeta) {
                    failedCount++;
                    if (failedCount >= 3 || delta >= 400) {
                        aspirBeta = SCORE_INFINITY;
                    } else {
                        delta = std::min(delta * 2, 400);
                        aspirBeta = std::min(SCORE_INFINITY, prevScore + delta);
                    }
                } else {
                    break; // Exact score within window
                }
            }
        }

        if (stopped && depth > 1) break;
        prevScore = score;

        // Extract best move from triangular PV table or TT
        if (pvLength[0] > 0 && pvTable[0][0].getMove() != 0) {
            result.bestMove = pvTable[0][0];
        } else {
            int ttMove = 0, dummy = 0;
            TT.Probe(pos->getPosKey(), depth, -SCORE_INFINITY, SCORE_INFINITY, ttMove, dummy, 0);
            if (ttMove != 0) result.bestMove.setMove(ttMove);
        }

        result.score = score;
        result.depth = depth;
        result.nodes = nodesCount;

        auto now        = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        result.timeMs   = durationMs;
        long long nps   = durationMs > 0 ? (nodesCount * 1000 / durationMs) : 0;

        // UCI info output
        std::cout << "info depth " << depth << " score ";
        if      (score >  SCORE_MATE - 1000) std::cout << "mate " << ( SCORE_MATE - score + 1) / 2;
        else if (score < -SCORE_MATE + 1000) std::cout << "mate " << (-SCORE_MATE - score) / 2;
        else                                  std::cout << "cp " << score;
        std::cout << " nodes " << nodesCount << " nps " << nps << " time " << durationMs << " pv ";

        if (pvLength[0] > 0) {
            for (int i = 0; i < pvLength[0]; i++) {
                std::cout << pvTable[0][i].GetMoveString() << " ";
            }
        } else {
            std::vector<Move> pv = GetPVLine(depth);
            for (const Move& m : pv) std::cout << m.GetMoveString() << " ";
        }
        std::cout << std::endl;

        // Soft time limit: stop if we've used more than 60% of allocated time
        if (limits.maxTimeMs > 0 && durationMs * 10 >= limits.maxTimeMs * 6) break;
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// PV Line Retrieval
// ─────────────────────────────────────────────────────────────────────────────
std::vector<Move> SearchEngine::GetPVLine(int maxDepth) {
    std::vector<Move> pv;
    if (pvLength[0] > 0) {
        for (int i = 0; i < pvLength[0] && i < maxDepth; i++) {
            pv.push_back(pvTable[0][i]);
        }
        return pv;
    }

    std::vector<U64> seen;
    for (int d = 0; d < maxDepth; d++) {
        U64 key = pos->getPosKey();
        if (std::find(seen.begin(), seen.end(), key) != seen.end()) break;
        seen.push_back(key);

        int moveVal = 0, sc = 0;
        if (!TT.Probe(key, 0, -SCORE_INFINITY, SCORE_INFINITY, moveVal, sc, d) || moveVal == 0) break;

        Move m; m.setMove(moveVal);
        if (!maker.MakeMoves(m)) break;
        pv.push_back(m);
    }
    for (size_t i = 0; i < pv.size(); i++) maker.TakeMove();
    return pv;
}
