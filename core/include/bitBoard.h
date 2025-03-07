#pragma once

#include <cstdint>

/*
We use each bit of the uint64 to define a square
*/

using bitboard = uint64_t;

// A square is just a number 0-63 denoting at what bit something is
using square = uint8_t;

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
    constexpr bool inBounds(int rank, int file)
    {
        return rank >= 0 && rank < 8 && file >= 0 && file < 8;
    }

    inline bool getBit(bitboard bb, int rank, int file)
    {
        return (bb >> (rank * 8 + file)) & 1;
    }

    inline void setBit(bitboard &bb, int rank, int file)
    {
        bb = bb | (1ULL << (rank * 8 + file));
    }

    inline void unSetBit(bitboard &bb, int rank, int file)
    {
        bb &= ~(1ULL << (rank * 8 + file));
    }

    constexpr bitboard rankMask(int rank)
    {
        bitboard rankMask = 0xFF;
        return rankMask << rank * 8;
    }

    constexpr bitboard fileMask(int file)
    {
        bitboard fileMask = 0x0101010101010101;
        return fileMask << file;
    }

    // 1's all on the edges of the board
    constexpr bitboard edgeMask = 0xFF818181818181FF;

    // Note that the output for 1 and 0 is the same!
    inline square firstSetBit(bitboard bb)
    {
        const uint64_t debruijn64 = 0x03f79d71b4cb0a89;
        return debruijnBitScanTable[((bb & -bb) * debruijn64) >> 58];
    }
}
