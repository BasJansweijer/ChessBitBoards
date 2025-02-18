#pragma once

#include <cstdint>

/*
We use each bit of the uint64 to define a square
*/

using bitboard = uint64_t;

// A square is just a number 0-63 denoting at what bit something is
using square = uint8_t;

// For debugging purposes
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

    int firstSetBit(bitboard bb);

}
