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
        mask = mask::oneStep(b.getWhitePawns());
        mask |= mask::oneStep(mask);
        return mask;
    }
}

int main()
{
    for (chess::square s1 = 0; s1 < 64; s1++)
    {
        for (chess::square s2 = 0; s2 < 64; s2++)
        {
            if (chess::mask::inBetween(s1, s2) == 0)
                continue;

            chess::bitBoards::showBitboardGUI(1ULL << s1 | 1ULL << s2);

            // Show the in between mask
            chess::bitBoards::showBitboardGUI(chess::mask::inBetween(s1, s2));
        }
    }
    testMaskOnFens("testing/fens10000.txt", chess::testMask);
}