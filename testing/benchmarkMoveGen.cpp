#include <iostream>
#include <string>
#include <chrono>
#include <fstream>

#include "chess.h"
#include "toolUtils.h"

int perft(chess::BoardState &b, int depth)
{
    if (depth == 0)
        return 1;

    int nodes = 1;

    for (auto m : b.pseudoLegalMoves())
    {
        chess::BoardState newB = b;
        newB.makeMove(m);
        if (newB.kingAttacked(!newB.whitesMove()))
            continue;

        nodes += perft(newB, depth - 1);
    }

    return nodes;
}

int benchMoveGen(std::string fensFile, int depth = 4)
{
    std::ifstream fens(fensFile);
    std::string fen;

    uint64_t searchedNodes = 0;

    while (getline(fens, fen))
    {
        chess::BoardState b(fen);
        {
            utils::Timer t;
            searchedNodes += perft(b, depth);
        }
    }

    int nodesPerSecond = searchedNodes / utils::Timer::getAccumulatedTime();

    return nodesPerSecond;
}

int main(int argc, char *argv[])
{
    // The quick mode is usefull for faster itteration when experimenting with optimizations
    bool quickMode = false;

    // Loop through command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--quick")
        {
            quickMode = true;
        }
    }

    std::string fensFile = quickMode ? "testing/fens10.txt" : "testing/fens10000.txt";

    int nps = benchMoveGen(fensFile);
    std::cout << "nps: " << nps << std::endl;
}