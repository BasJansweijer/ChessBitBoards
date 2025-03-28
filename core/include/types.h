#pragma once

#include <cinttypes>

namespace chess
{
    enum PieceType
    {
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King,
        None = -1
    };

    /*
    We use each bit of the uint64 to define a square
    */

    using bitboard = uint64_t;

    // A square is just a number 0-63 denoting at what bit something is
    using square = uint8_t;

    // zobrist hash type
    using key = uint64_t;

}