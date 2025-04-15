#include <fstream>
#include <iostream>
#include <string>
#include <functional>

#include "chess.h"
#include "bitBoard.h"
#include "boardVisualizer.h"
#include "masks.h"

void testMaskOnFens(std::string fensPath, std::function<chess::bitboard(const chess::BoardState &)> mask)
{
    std::ifstream fensFile(fensPath);
    std::string fen;
    while (getline(fensFile, fen))
    {
        chess::BoardState b(fen);

        chess::showBoardGUI(b, mask(b));
    }
}

chess::bitboard testMask(const chess::BoardState &b)
{
    return chess::mask::fillSouth(b.getWhitePawns());
}

int main()
{
    testMaskOnFens("testing/fens10000.txt", testMask);
}