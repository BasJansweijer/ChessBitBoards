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
    for (chess::square s = 0; s < 64; s++)
    {
        chess::bitBoards::showBitboardGUI(chess::mask::pawnSquare<true>(s));

        chess::bitBoards::showBitboardGUI(chess::mask::pawnSquare<false>(s));
        }
    testMaskOnFens("testing/fens10000.txt", chess::testMask);
}