#pragma once

#include <cstdint>

using bitboard = uint64_t;

/*
We use each bit of the uint64 to define a square
*/

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
}