#include "bitBoard.h"
#include "types.h"

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

}