#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"

namespace chess
{
    bitboard BoardState::allPieces(bool white) const
    {
        return white ? m_white.allPieces() : m_black.allPieces();
    }

    bitboard BoardState::allPieces() const
    {
        return m_white.allPieces() | m_black.allPieces();
    }

    bool BoardState::squareAttacked(square s, bool byWhite) const
    {
        // bitboards of all the pieces that could attack the square
        const PieceSet &attacker = byWhite ? m_white : m_black;

        bitboard allBlockers = allPieces();

        int8_t rank = s / 8;
        int8_t file = s % 8;
        int8_t pawnRank = byWhite ? rank - 1 : rank + 1;

        square pawnAttackRight = file != 7 ? pawnRank * 8 + file + 1 : -1;
        square pawnAttackLeft = file != 0 ? pawnRank * 8 + file - 1 : -1;

        return attacker.pawns & 1ULL << pawnAttackLeft ||
               attacker.pawns & 1ULL << pawnAttackRight ||
               constants::knightMoves[s] & attacker.knights ||
               constants::kingMoves[s] & attacker.king ||
               constants::getBishopMoves(s, allBlockers) & (attacker.bishops | attacker.queens) ||
               constants::getRookMoves(s, allBlockers) & (attacker.rooks | attacker.queens);
    }

    bool BoardState::kingAttacked(bool white) const
    {
        square kingPos = bitBoards::firstSetBit(white ? m_white.king : m_black.king);
        return squareAttacked(kingPos, !white);
    }
}