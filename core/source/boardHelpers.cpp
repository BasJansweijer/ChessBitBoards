#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"

namespace chess
{
    bitboard BoardState::allPieces(bool white) const
    {
        if (white)
            return m_white.pawns | m_white.knights |
                   m_white.bishops | m_white.rooks |
                   m_white.queens | m_white.king;
        else
            return m_black.pawns | m_black.knights |
                   m_black.bishops | m_black.rooks |
                   m_black.queens | m_black.king;
    }

    bitboard BoardState::allPieces() const
    {
        return m_white.pawns | m_white.knights |
               m_white.bishops | m_white.rooks |
               m_white.queens | m_white.king |
               m_black.pawns | m_black.knights |
               m_black.bishops | m_black.rooks |
               m_black.queens | m_black.king;
    }

    bool BoardState::squareAttacked(square s, bool byWhite) const
    {
        // bitboards of all the pieces that could attack the square

        const bitboard &pawns = byWhite ? m_white.pawns : m_black.pawns;
        const bitboard &knights = byWhite ? m_white.knights : m_black.knights;
        const bitboard &bishops = byWhite ? m_white.bishops : m_black.bishops;
        const bitboard &rooks = byWhite ? m_white.rooks : m_black.rooks;
        const bitboard &queens = byWhite ? m_white.queens : m_black.queens;
        const bitboard &king = byWhite ? m_white.king : m_black.king;
    }
}