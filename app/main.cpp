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

int main()
{
    chess::BoardState b;
    b.makeMove(chess::Move(8, 24, chess::Pawn, false));
    b.makeMove(chess::Move(8 * 6 + 5, 8 * 5 + 5, chess::Pawn, false));

    b.makeMove(chess::Move(8 * 3, 8 * 4, chess::Pawn, false));

    b.makeMove(chess::Move(8 * 6 + 1, 8 * 4 + 1, chess::Pawn, false));
    randomMoves(b);
}
