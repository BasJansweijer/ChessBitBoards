#include <iostream>
#include <string>
#include <vector>
#include "chess.h"
#include "stockfish.h"
#include "boardVisualizer.h"
#include <fstream>
#include <thread>
#include <chrono>

#define GREEN "\033[32m"

uint64_t countLeafNodes(chess::BoardState &b, int depth)
{
    if (depth == 0)
    {
        return 1;
    }

    uint64_t leafNodes = 0;

    for (auto move : b.legalMoves())
    {
        chess::BoardState bNew = b;
        bNew.makeMove(move);

        // if king is under attack on opponents move thats illegal
        if (bNew.kingAttacked(!bNew.whitesMove()))
            continue;

        leafNodes += countLeafNodes(bNew, depth - 1);
    }

    return leafNodes;
}

struct MoveListDifference
{
    // Moves stockfish finds but are not in our moves
    std::vector<std::string> notFoundMoves;
    std::vector<chess::Move> illegalMoves;

    bool noDiference()
    {
        return notFoundMoves.size() == 0 && illegalMoves.size() == 0;
    }

    void printDifference()
    {
        std::cout << "Did not produce the following moves: ";
        for (auto &m : notFoundMoves)
            std::cout << m << ' ';

        std::cout << "\n\n Produced illegal moves: ";
        for (auto &m : illegalMoves)
            std::cout << m.toUCI() << ' ';
        std::cout << std::endl;
    }
};

MoveListDifference getDifference(stockfish::perftResult stockfishMoves, chess::MoveList moves)
{
    MoveListDifference result;
    for (auto &[uciMove, i] : stockfishMoves)
    {
        bool moveFound = false;
        for (const chess::Move &m : moves)
        {
            if (m.toUCI() == uciMove)
            {
                moveFound = true;
                break;
            }
        }

        if (!moveFound)
        {
            result.notFoundMoves.push_back(uciMove);
        }
    }

    for (const chess::Move &m : moves)
    {
        bool moveFound = false;
        for (auto &[uciMove, i] : stockfishMoves)
        {
            if (m.toUCI() == uciMove)
            {
                moveFound = true;
                break;
            }
        }

        if (!moveFound)
        {
            result.illegalMoves.push_back(m);
        }
    }

    return result;
}

static stockfish::Engine engine;

bool testMoveGenCorrectness(chess::BoardState &b, int depth)
{
    if (depth == 0)
    {
        return false;
    }

    bool foundDiscrepency = false;

    engine.setPosition(b.fen());
    stockfish::perftResult expectedRes = engine.perft(depth);

    uint64_t leafNodes = 0;

    chess::MoveList legalMoves = b.legalMoves();

    // We encountered a move with an incorrect count after it.
    auto diff = getDifference(expectedRes, legalMoves);
    if (!diff.noDiference())
    {
        foundDiscrepency = true;
        diff.printDifference();
        chess::showBoardGUI(b);
    }

    for (auto move : legalMoves)
    {

        chess::BoardState bNew = b;
        bNew.makeMove(move);

        int countAfterMove = countLeafNodes(bNew, depth - 1);
        leafNodes += countAfterMove;

        if (expectedRes[move.toUCI()] == countAfterMove)
        {
            continue;
        }

        foundDiscrepency = true;
        std::cout << "Discrepency after: " << move.toUCI() << std::endl;
        std::cout << "expected: " << expectedRes[move.toUCI()] << " got: " << countAfterMove << std::endl;
        chess::showBoardGUI(bNew);

        testMoveGenCorrectness(bNew, depth - 1);
    }

    return !foundDiscrepency;
}

void showTestFens()
{
    std::ifstream fens_file("testing/fens.txt");

    std::string fen;
    while (getline(fens_file, fen))
    {
        chess::BoardState b(fen);
        chess::showBoardGUI(b);
    }
}

void updateProgress(int progress)
{
    int numChars = 4;
    for (int i = 0; i < numChars; i++)
        std::cout << '\b';

    std::string text = std::to_string(progress);
    text += '%';
    while (text.size() < numChars)
        text += ' ';
    std::cout << text;
    std::cout.flush();
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
        bool correct = testMoveGenCorrectness(b, testDepth);
        if (!correct)
        {
            std::cout << "failed on fen:\n"
                      << fen << std::endl;
            break;
        }

        fensTested++;
        progress = 100 * fensTested / numFens;
        updateProgress(progress);
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
