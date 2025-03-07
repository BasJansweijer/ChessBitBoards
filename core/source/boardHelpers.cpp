#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"

namespace chess
{

    bool BoardState::squareAttacked(square s, bool byWhite) const
    {
        // bitboards of all the pieces that could attack the square
        const PieceSet &attacker = byWhite ? m_white : m_black;

        bitboard allBlockers = allPieces();
        int8_t file = s % 8;

        // compute the square above/bellow square s (on rank from which pawns can attack)
        square pawnRank = s - (byWhite ? 8 : -8);
        square pawnAttackRight = pawnRank + 1;
        square pawnAttackLeft = pawnRank - 1;

        // We check if we aren't on the edge of the board since in that case
        // the pawn attack square doesn't exist
        return (file != 0 && attacker.pawns & 1ULL << pawnAttackLeft) ||
               (file != 7 && attacker.pawns & 1ULL << pawnAttackRight) ||
               constants::knightMoves[s] & attacker.knights ||
               constants::kingMoves[s] & 1ULL << attacker.king ||
               constants::getBishopMoves(s, allBlockers) & (attacker.bishops | attacker.queens) ||
               constants::getRookMoves(s, allBlockers) & (attacker.rooks | attacker.queens);
    }

    bool BoardState::kingAttacked(bool white) const
    {
        return squareAttacked(white ? m_white.king : m_black.king, !white);
    }
}