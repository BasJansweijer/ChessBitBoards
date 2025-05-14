#include "bitBoard.h"
#include "types.h"
#include <stdint.h>

namespace chess::mask
{
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

    inline bitboard fillNorth(bitboard bb)
    {
        // Fill the north direction
        bb |= bb << 8;
        bb |= bb << 16;
        bb |= bb << 32;
        return bb;
    }

    inline bitboard fillSouth(bitboard bb)
    {
        // Fill the south direction
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return bb;
    }

    template <bool isWhite>
    constexpr bitboard passedPawn(square s)
    {
        uint8_t rank = s / 8;
        uint8_t file = s % 8;

        bitboard mask = 0;
        // Three files that can stop the pawn
        mask |= mask::fileMask(file);
        mask |= file > 0 ? mask::fileMask(file - 1) : 0;
        mask |= file < 7 ? mask::fileMask(file + 1) : 0;

        // Shift up or down depending on the color
        uint8_t shift = 8 * (rank + (isWhite ? 1 : 0));
        isWhite ? mask <<= shift : mask >>= -shift;
        return mask;
    }

    // returns the mask of the squares that can be attacked by a specific pawn
    template <bool isWhite>
    constexpr bitboard pawnAttack(square pawnSquare)
    {
        uint8_t rank = pawnSquare / 8;
        uint8_t file = pawnSquare % 8;

        square inFront = isWhite ? pawnSquare + 8 : pawnSquare - 8;

        bitboard leftAttack = file > 0 ? 1ULL << inFront - 1 : 0;
        bitboard rightAttack = file < 7 ? 1ULL << inFront + 1 : 0;
        bitboard mask = leftAttack | rightAttack;
        return mask;
    }

    // returns the mask of attacked squares by all the given pawns
    template <bool isWhite>
    constexpr bitboard pawnAttacks(bitboard pawns)
    {
        bitboard leftAttack;
        bitboard rightAttack;
        if constexpr (isWhite)
        {
            leftAttack = (pawns & ~mask::fileMask(0)) << (8 - 1);
            rightAttack = (pawns & ~mask::fileMask(7)) << (8 + 1);
        }
        else
        {
            leftAttack = (pawns & ~mask::fileMask(0)) >> (8 + 1);
            rightAttack = (pawns & ~mask::fileMask(7)) >> (8 - 1);
        }

        bitboard mask = leftAttack | rightAttack;

        return mask;
    }

    template <bool isWhite>
    constexpr bitboard pawnAttackSpan(bitboard pawns)
    {
        bitboard attacks = pawnAttacks<isWhite>(pawns);

        // Fill the attack span
        auto fill = isWhite ? fillNorth : fillSouth;
        bitboard attackSpan = fill(attacks);
        return attackSpan;
    }

    template <bool isWhite>
    constexpr bitboard backwardPawns(bitboard ourPawns, bitboard enemyPawns)
    {
        bitboard ourAttackSpan = pawnAttackSpan<isWhite>(ourPawns);

        bitboard enemyAttacks = pawnAttacks<!isWhite>(enemyPawns);
        // Spots we can't defend with our pawns but that are attacked by opponent
        bitboard stopSquares = (~ourAttackSpan) & enemyAttacks;

        auto enemyFill = isWhite ? fillSouth : fillNorth;
        bitboard backwardArea = enemyFill(stopSquares);

        return backwardArea & ourPawns;
    }

    struct InBetweenTable
    {
        bitboard table[64][64];

        constexpr InBetweenTable()
        {
            for (square s1 = 0; s1 < 64; s1++)
                for (square s2 = 0; s2 < 64; s2++)
                    table[s1][s2] = inBetweenHelper(s1, s2);
        }

        constexpr static bitboard inBetweenHelper(square s1, square s2)
        {
            if (s1 == s2)
                return 0;

            uint8_t s1File = s1 % 8;
            uint8_t s1Rank = s1 / 8;
            uint8_t s2File = s2 % 8;
            uint8_t s2Rank = s2 / 8;
            int8_t fileDiff = s2File - s1File;
            int8_t rankDiff = s2Rank - s1Rank;

            bool isStaight = fileDiff == 0 || rankDiff == 0;
            bool isDiagonal = abs(fileDiff) == abs(rankDiff);

            if (!isStaight && !isDiagonal)
                return 0;

            bitboard mask = 0;
            int8_t fileStep = fileDiff == 0 ? 0 : (fileDiff > 0 ? 1 : -1);
            int8_t rankStep = rankDiff == 0 ? 0 : (rankDiff > 0 ? 1 : -1);

            square currentSquare = s1;
            while (currentSquare != s2)
            {
                currentSquare += fileStep + rankStep * 8;
                mask |= 1ULL << currentSquare;
            }

            // Remove the start and end squares
            mask ^= 1ULL << s2;

            return mask;
        }
    };

    constexpr InBetweenTable inBetweenTable;

    constexpr bitboard inBetween(square s1, square s2)
    {
        return inBetweenTable.table[s1][s2];
    }

    // Returns a bitboard of all locations that were a single king move
    // away from the given squares
    constexpr bitboard oneStep(bitboard bb)
    {
        bb |= bb << 8; // up
        bb |= bb >> 8; // down
        // we now have the up and down as 1's

        bitboard left = (bb & ~mask::fileMask(0)) >> 1;
        bitboard right = (bb & ~mask::fileMask(7)) << 1;

        return bb | left | right;
    }

    // Assumes that the pawn given is white if isWhite true.
    // Also assumes that the current move is the player with the pawn
    // (if not you can shift the pawn one more square)
    template <bool isWhite>
    constexpr bitboard pawnSquare(square pawn)
    {
        uint8_t rank = pawn / 8;
        uint8_t stepsUntillQueening = isWhite ? 7 - rank : rank;

        bitboard squareMask = 1ULL << pawn;
        bitboard squareBorder = rankMask(isWhite ? rank - 1 : rank + 1);

        for (int i = 0; i < stepsUntillQueening; i++)
        {
            squareMask = oneStep(squareMask);
            squareMask &= ~squareBorder;
        }

        return squareMask;
    }
}
