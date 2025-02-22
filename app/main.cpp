#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>
#include <string>
#include <random>
#include <vector>

void testFen(std::string_view fen)
{

    chess::BoardState b(fen);
    std::cout << b.canWhiteCastleShort()
              << b.CanWhiteCastleLong()
              << b.canBlackCastleShort()
              << b.CanBlackCastleLong() << std::endl;

    chess::showBoardGUI(b, b.getEnpassentLocations());
}

void randomMoves(chess::BoardState &b)
{
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister PRNG
    std::uniform_int_distribution<size_t> dist(0, 100);

    int i = 0;
    while (i < 100)
    {
        chess::showBoardGUI(b);
        std::vector<chess::Move> possibleMoves = b.legalMoves();
        b.makeMove(possibleMoves[dist(gen) % possibleMoves.size()]);
        i++;
    }
    dist(gen);
}

static int exploredPositions = 0;
static int leafNodePositions = 0;
static int maxDepth = 6;
static int checkmates = 0;

void exploreAll(chess::BoardState &b, int depth = 0)
{
    exploredPositions++;
    if (depth >= maxDepth)
    {
        leafNodePositions++;
        return;
    }
    bool noLegalMoves = true;
    for (auto m : b.legalMoves())
    {
        chess::BoardState bNew = b;
        bNew.makeMove(m);

        // if king is under attack on opponents move thats illegal
        if (bNew.kingAttacked(!bNew.whitesMove()))
            continue;

        noLegalMoves = false;

        exploreAll(bNew, depth + 1);
    }

    if (noLegalMoves)
        checkmates++;
}

int main()
{
    chess::BoardState b;
    chess::showBoardGUI(b);
    exploreAll(b);
    std::cout << "explored: " << exploredPositions << std::endl;
    std::cout << "found checkmates: " << checkmates << std::endl;
    std::cout << "positions after the moves: " << leafNodePositions << std::endl;
}
