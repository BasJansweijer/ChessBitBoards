#include "bitBoard.h"
#include <iostream>

namespace chess::bitBoards
{

    void printBitboard(bitboard bb)
    {
        // first print the last rank (blacks side)
        for (int rank = 7; rank >= 0; rank--)
        {
            for (int file = 0; file < 8; file++)
            {
                int bit = (bb >> (rank * 8 + file)) & 1;
                std::cout << bit;
            }
            std::cout << '\n';
        }
        std::cout << std::endl;
    }
}