#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include "chess.h"
#include "bitBoard.h"
#include "boardVisualizer.h"
#include "masks.h"

namespace chess
{
    void testMaskOnFens(std::string fensPath, std::function<bitboard(const BoardState &)> mask)
    {
        std::ifstream fensFile(fensPath);
        std::string fen;
        while (getline(fensFile, fen))
        {
            chess::BoardState b(fen);
            chess::showBoardGUI(b, mask(b));
        }
    }

    bitboard allPawnAttacks(const BoardState &b)
    {
        bitboard mask = 0;
        mask |= mask::pawnAttacks<true>(b.getWhitePawns());
        mask |= mask::pawnAttacks<false>(b.getBlackPawns());
        return mask;
    }

    bitboard testMask(const BoardState &b)
    {
        bitboard mask = 0;

        // mask |= mask::pawnAttacks<false>(b.getBlackPawns());
        mask |= mask::backwardPawns<true>(b.getWhitePawns(), b.getBlackPawns());
        return mask;
    }
}

int main()
{
    testMaskOnFens("testing/fens10000.txt", chess::testMask);
}