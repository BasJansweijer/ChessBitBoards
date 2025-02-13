#pragma once

#include <cstdint>

using bitboard = uint64_t;

/*
We use each bit of the uint64 to define a square
*/

// For debugging purposes
namespace chess::bitBoards
{
    void printBitboard(bitboard bb);
}