#include "bitBoard.h"
#include <iostream>

/**
 * bitScanForward
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */

constexpr static square debruijnBitScanTable[64] = {
    0, 1, 48, 2, 57, 49, 28, 3,
    61, 58, 50, 42, 38, 29, 17, 4,
    62, 55, 59, 36, 53, 51, 43, 22,
    45, 39, 33, 30, 24, 18, 12, 5,
    63, 47, 56, 27, 60, 41, 37, 16,
    54, 35, 52, 21, 44, 32, 23, 11,
    46, 26, 40, 15, 34, 20, 31, 10,
    25, 14, 19, 9, 13, 8, 7, 6};

namespace chess::bitBoards
{
    // Note that the output for 1 and 0 is the same!
    square firstSetBit(bitboard bb)
    {
        const uint64_t debruijn64 = 0x03f79d71b4cb0a89;
        return debruijnBitScanTable[((bb & -bb) * debruijn64) >> 58];
    }

}