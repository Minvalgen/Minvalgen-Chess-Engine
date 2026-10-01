#include <iostream>
#include <chrono>
#include <string>

#include "Defs.h"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "MakeMove.hpp"
#include "Perft.hpp"
#include "Search.hpp"
#include "UCI.hpp"

void RunBenchmark() {
    InitAll();
    Board board;

    std::cout << "=======================================\n";
    std::cout << " Minvalgen Search Engine Benchmark\n";
    std::cout << "=======================================\n";

    std::string testPositions[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // Start pos
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", // Kiwipete
        "2r2rk1/1bqnbppp/1p1ppn2/pP6/2P1P3/1PN2NP1/5PBP/R1BQR1K1 b - - 0 14",
        "r1bqkb1r/pppp1ppp/2n5/4p3/2B1n3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 0 1" // Tactical Mate position
    };

    SearchEngine engine(&board);

    for (int i = 0; i < 4; i++) {
        std::cout << "\nTest Position " << (i + 1) << ": " << testPositions[i] << "\n";
        board.Parse(testPositions[i]);

        SearchLimits limits;
        limits.maxDepth = 8;

        SearchResult res = engine.SearchPosition(limits);
        std::cout << "Best Move: ";
        res.bestMove.PrtAgeMove();
        std::cout << "Score: " << res.score << " cp | Depth: " << res.depth
                  << " | Nodes: " << res.nodes << " | Time: " << res.timeMs << " ms\n";
    }
}

void RunPerft(int depth) {
    InitAll();
    Board board;
    board.Parse(FEN_STARTUP);

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
}

int main(int argc, char* argv[]) {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    if (argc > 1 && std::string(argv[1]) == "bench") {
        RunBenchmark();
        return 0;
    }

    if (argc > 2 && std::string(argv[1]) == "perft") {
        int depth = std::stoi(argv[2]);
        RunPerft(depth);
        return 0;
    }

    UCI uci;
    uci.Loop();
    return 0;
}