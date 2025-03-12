#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"

namespace chess
{
    template <bool ByWhite>
    bool BoardState::squareAttacked(square s) const
    {
        // bitboards of all the pieces that could attack the square
        const bitboard *attacker = ByWhite ? m_whitePieces : m_blackPieces;
        const square attackerKing = ByWhite ? m_whiteKing : m_blackKing;

        bitboard allBlockers = allPieces();
        int8_t file = s % 8;

        // compute the square above/bellow square s (on rank from which pawns can attack)
        square pawnRank = s - (ByWhite ? 8 : -8);
        square pawnAttackRight = pawnRank + 1;
        square pawnAttackLeft = pawnRank - 1;

        // We check if we aren't on the edge of the board since in that case
        // the pawn attack square doesn't exist
        return (file != 0 && attacker[PieceType::Pawn] & 1ULL << pawnAttackLeft) ||
               (file != 7 && attacker[PieceType::Pawn] & 1ULL << pawnAttackRight) ||
               constants::knightMoves[s] & attacker[PieceType::Knight] ||
               constants::kingMoves[s] & 1ULL << attackerKing ||
               constants::getBishopMoves(s, allBlockers) & (attacker[PieceType::Bishop] | attacker[PieceType::Queen]) ||
               constants::getRookMoves(s, allBlockers) & (attacker[PieceType::Rook] | attacker[PieceType::Queen]);
    }

    template bool BoardState::squareAttacked<true>(square s) const;
    template bool BoardState::squareAttacked<false>(square s) const;

    bool BoardState::kingAttacked(bool white) const
    {
        return white ? squareAttacked<false>(m_whiteKing) : squareAttacked<true>(m_blackKing);
    }
}