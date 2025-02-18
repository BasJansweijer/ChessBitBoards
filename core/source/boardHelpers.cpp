#include "chess.h"

namespace chess
{
    bitboard BoardState::allPieces(bool white) const
    {
        if (white)
            return m_whitePawns | m_whiteKnights |
                   m_whiteBishops | m_whiteRooks |
                   m_whiteQueens | m_whiteKing;
        else
            return m_blackPawns | m_blackKnights |
                   m_blackBishops | m_blackRooks |
                   m_blackQueens | m_blackKing;
    }

    bitboard BoardState::allPieces() const
    {
        return m_whitePawns | m_whiteKnights |
               m_whiteBishops | m_whiteRooks |
               m_whiteQueens | m_whiteKing |
               m_blackPawns | m_blackKnights |
               m_blackBishops | m_blackRooks |
               m_blackQueens | m_blackKing;
    }
}