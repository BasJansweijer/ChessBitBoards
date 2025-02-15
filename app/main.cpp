#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>
#include <string>

void testFen(std::string_view fen)
{

    chess::BoardState b(fen);
    std::cout << b.canWhiteCastleShort()
              << b.CanWhiteCastleLong()
              << b.canBlackCastleShort()
              << b.CanBlackCastleLong() << std::endl;

    chess::showBoardGUI(b, b.getEnpassentLocations());
}

int main()
{
    testFen("4r3/2P3R1/R1N2k1P/5Np1/K1p1p3/1pr5/3P4/Bn3Q2 w - - 0 1");
    testFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}
