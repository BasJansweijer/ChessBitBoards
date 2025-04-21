#pragma once

#include <cinttypes>

namespace chess
{
    enum PieceType : int8_t
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

    // eval score
    using score = int16_t;
#define SCORE_MAX INT16_MAX
#define SCORE_MIN INT16_MIN

    // The minimum int that is still forced mate
    constexpr score MIN_MATE_SCORE = 30000;
    constexpr int MAX_DEPTH = 1000;
    constexpr score MAX_MATE_SCORE = MIN_MATE_SCORE + MAX_DEPTH;

    // The exact values correspond to the flags in the TTEntries
    enum EvalBound : uint8_t
    {
        Lower = 0b1000000,
        Upper = 0b10000000,
        Exact = Lower | Upper
    };
}