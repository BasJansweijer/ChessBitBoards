#include <iostream>
#include <string>
#include <fstream>

#include "chess.h"
#include "toolUtils.h"
#include "types.h"
#include "boardVisualizer.h"
#include "zobristHash.h"

void compareIncrementalZobrist(chess::BoardState &b, int depth)
{
    if (depth == 0)
        return;

    constexpr chess::BoardState::MoveGenType NORMAL = chess::BoardState::MoveGenType::Normal;

    for (auto m : b.pseudoLegalMoves<NORMAL>())
    {
        chess::BoardState newB = b;

        chess::key prevKey = newB.getHash();
        newB.makeMove(m);

        chess::key incKey = newB.getHash();
        newB.recomputeHash();

        if (newB.getHash() != incKey)
        {
            std::cout << "Issue after: " << m.toUCI() << std::endl;
            bitboard move = 1ULL << m.to | 1ULL << m.from;

            int pieceIdx = m.piece + b.whitesMove() ? 0 : 6;
            prevKey ^= chess::zobrist::squarePieceKeys[m.to][pieceIdx];
            prevKey ^= chess::zobrist::squarePieceKeys[m.from][pieceIdx];

            std::cout << "Manual: " << prevKey << std::endl;
            chess::showBoardGUI(b, move);
        }

        if (newB.kingAttacked(!newB.whitesMove()))
            continue;

        compareIncrementalZobrist(newB, depth - 1);
    }

    return;
}

int numTestFens(std::string fensPath)
{
    std::ifstream fensFile(fensPath);

    return std::count(std::istreambuf_iterator<char>(fensFile),
                      std::istreambuf_iterator<char>(), '\n');
}

void checkTestFens(int testDepth, std::string fensPath)
{

    int numFens = numTestFens(fensPath);

    std::ifstream fensFile(fensPath);
    std::cout << "Starting movegeneration test on " << numFens << " positions " << std::endl;
    int progress = 0.0;
    int fensTested = 0;

    std::string fen;
    while (getline(fensFile, fen))
    {
        chess::BoardState b(fen);
        compareIncrementalZobrist(b, testDepth);
        fensTested++;
    }
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

    checkTestFens(4, fensFile);
}