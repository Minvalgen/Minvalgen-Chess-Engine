#include "UCI.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include "Perft.hpp"

std::string UCI::GetBookMove(const std::string& moves) {
    std::vector<std::string> bookLines = {
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6",
        "e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6",
        "e2e4 e7e6 d2d4 d7d5 b1c3 g8f6",
        "e2e4 c7c6 d2d4 d7d5 b1c3 d5e4",
        "d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5",
        "d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5",
        "d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 g2g3 c8b7",
        "c2c4 e7e5 b1c3 g8f6 g1f3 b8c6"
    };
    
    std::vector<std::string> candidates;
    std::string prefix = moves.empty() ? "" : moves + " ";
    
    for (const auto& line : bookLines) {
        if (line.find(prefix) == 0) {
            std::string remaining = line.substr(prefix.length());
            size_t space = remaining.find(' ');
            if (space != std::string::npos) {
                candidates.push_back(remaining.substr(0, space));
            } else if (!remaining.empty()) {
                candidates.push_back(remaining);
            }
        }
    }
    
    if (candidates.empty()) return "";
    
    srand(time(0));
    return candidates[rand() % candidates.size()];
}

Move UCI::ParseMoveString(const std::string& moveStr) {
    if (moveStr.length() < 4) return Move();

    int fromFile = moveStr[0] - 'a';
    int fromRank = moveStr[1] - '1';
    int toFile = moveStr[2] - 'a';
    int toRank = moveStr[3] - '1';

    int fromSq = GetSquare(fromFile, fromRank);
    int toSq = GetSquare(toFile, toRank);

    MoveGenerator gen(&board);
    gen.GenerateAllMoves();

    for (int i = 0; i < gen.getCountMoves(); i++) {
        Move m = gen.getMove(i);
        if (m.FromSq() == fromSq && m.ToSq() == toSq) {
            int pro = m.Promoted();
            if (moveStr.length() == 5) {
                char pchar = moveStr[4];
                if (pchar == 'q' && (pro == wQ || pro == bQ)) return m;
                if (pchar == 'r' && (pro == wR || pro == bR)) return m;
                if (pchar == 'b' && (pro == wB || pro == bB)) return m;
                if (pchar == 'n' && (pro == wN || pro == bN)) return m;
                continue;
            }
            return m;
        }
    }

    return Move();
}

void UCI::ParsePosition(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token; // "position"

    ss >> token;
    if (token == "startpos") {
        isStartPos = true;
        board.Parse(FEN_STARTUP);
        ss >> token; // Check if "moves" follow
    } else if (token == "fen") {
        isStartPos = false;
        std::string fenStr = "";
        while (ss >> token && token != "moves") {
            if (!fenStr.empty()) fenStr += " ";
            fenStr += token;
        }
        board.Parse(fenStr);
    }

    currentMoves = "";
    if (token == "moves") {
        MakeMove engine(&board);
        while (ss >> token) {
            Move m = ParseMoveString(token);
            if (m.getMove() != 0) {
                engine.MakeMoves(m);
                if (!currentMoves.empty()) currentMoves += " ";
                currentMoves += token;
            }
        }
    }
}

void UCI::ParseGo(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    ss >> token; // "go"

    SearchLimits limits;
    long long wtime = -1, btime = -1, winc = 0, binc = 0, movetime = -1;
    int depth = 64;

    while (ss >> token) {
        if (token == "depth") ss >> depth;
        else if (token == "wtime") ss >> wtime;
        else if (token == "btime") ss >> btime;
        else if (token == "winc") ss >> winc;
        else if (token == "binc") ss >> binc;
        else if (token == "movetime") ss >> movetime;
    }

    limits.maxDepth = depth;

    if (movetime > 0) {
        limits.maxTimeMs = movetime;
        limits.optimalTimeMs = movetime;
    } else if (wtime > 0 || btime > 0) {
        long long time = (board.getSide() == WHITE) ? wtime : btime;
        long long inc = (board.getSide() == WHITE) ? winc : binc;

        // Optimal time: normal thinking budget
        long long optimal = time / 20 + (inc * 3) / 4;
        // Hard limit: absolute maximum (for difficult positions)
        long long hard = std::min(optimal * 3, time / 3);

        if (optimal < 10) optimal = 10;
        if (hard < optimal) hard = optimal;

        limits.optimalTimeMs = optimal;
        limits.maxTimeMs = hard;
    }

    if (isStartPos) {
        std::string bookMove = GetBookMove(currentMoves);
        if (!bookMove.empty()) {
            Move bm = ParseMoveString(bookMove);
            if (bm.getMove() != 0) {
                std::cout << "bestmove " << bookMove << std::endl;
                return;
            }
        }
    }

    SearchEngine engine(&board);
    SearchResult res = engine.SearchPosition(limits);

    std::cout << "bestmove " << res.bestMove.GetMoveString() << std::endl;
}

void UCI::Loop() {
    InitAll();
    board.Parse(FEN_STARTUP);

    std::string line;
    std::cout << NAME << " by DeepMind Team" << std::endl;

    while (std::getline(std::cin, line)) {
        // Strip trailing \r if present
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line == "uci") {
            std::cout << "id name " << NAME << std::endl;
            std::cout << "id author DeepMind Team" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (line == "ucinewgame") {
            TT.Clear();
            board.Parse(FEN_STARTUP);
        } else if (line.rfind("position", 0) == 0) {
            ParsePosition(line);
        } else if (line.rfind("go", 0) == 0) {
            ParseGo(line);
        } else if (line.rfind("perft", 0) == 0) {
            std::stringstream ss(line);
            std::string token;
            int depth = 5;
            ss >> token; // "perft"
            if (ss >> token) {
                depth = std::stoi(token);
            }
            
            std::cout << "Starting perft test to depth " << depth << "...\n";
            auto start = std::chrono::high_resolution_clock::now();
            
            Perft perft(&board);
            perft.PerfTest(depth);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            long long nodes = perft.getNumMoves();
            double seconds = duration / 1000.0;
            long long nps = (seconds > 0) ? (nodes / seconds) : 0;
            
            std::cout << "Depth: " << depth << "\n";
            std::cout << "Nodes: " << nodes << "\n";
            std::cout << "Time : " << duration << " ms\n";
            std::cout << "NPS  : " << nps << " nodes/sec\n";
        } else if (line == "quit") {
            break;
        }
    }
}

