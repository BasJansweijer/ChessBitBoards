#include <iostream>
#include <string>
#include <vector>
#include "chess.h"
#include "stockfish.h"
#include "boardVisualizer.h"

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

MoveListDifference getDifference(stockfish::perftResult stockfishMoves, std::vector<chess::Move> moves)
{
    MoveListDifference result;
    for (auto &[uciMove, i] : stockfishMoves)
    {
        bool moveFound = false;
        for (chess::Move &m : moves)
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

    for (chess::Move &m : moves)
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

int findIncorrectMoveGen(chess::BoardState &b, int depth)
{
    if (depth == 0)
    {
        return 1;
    }

    static bool foundDiscrepency = false;

    engine.setPosition(b.fen());
    stockfish::perftResult expectedRes = engine.perft(depth);

    uint64_t leafNodes = 0;

    std::vector<chess::Move> legalMoves = b.legalMoves();

    // We encountered a move with an incorrect count after it.
    auto diff = getDifference(expectedRes, legalMoves);
    if (!diff.noDiference())
    {
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

        findIncorrectMoveGen(bNew, depth - 1);
    }

    if (!foundDiscrepency)
        std::cout << GREEN << "No incorrect move generation found!" << std::endl;

    return leafNodes;
}

int main()
{
    int searchDepth = 7;

    std::string start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // chess::BoardState b("rnbqkb1r/pppppp1p/7n/6pP/8/8/PPPPPPP1/RNBQKBNR/ w KQkq g6");

    chess::BoardState b;
    // findIncorrectMoveGen(b, searchDepth);

    // https://en.wikipedia.org/wiki/Shannon_number
    uint64_t leafNodes = countLeafNodes(b, searchDepth);

    std::cout << "Our leafNode count: " << leafNodes << std::endl;
}
