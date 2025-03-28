#include "chess.h"
#include "bitBoard.h"
#include "zobristHash.h"

#include <iostream>

namespace chess
{
    void BoardState::recomputeHash()
    {
        m_hash = 0;

        // Add the turn
        if (m_whitesMove)
            m_hash ^= zobrist::turnKey;

        // Add the castling rights
        m_hash ^= zobrist::castlingKeys[m_castleRights];

        // add enpassent square
        m_hash ^= zobrist::getEnpassentKey(m_enpassentSquare);

        // Add the pieces of both colors
        constexpr int blackOffset = 6;
        for (int pt = 0; pt < 5; pt++)
        {
            bitBoards::forEachBit(m_whitePieces[pt], [&](square s)
                                  { m_hash ^= zobrist::getPieceKey(s, (PieceType)pt, true); });

            bitBoards::forEachBit(m_blackPieces[pt], [&](square s)
                                  { m_hash ^= zobrist::getPieceKey(s, (PieceType)pt, false); });
        }

        // Add kings
        m_hash ^= zobrist::squarePieceKeys[m_whiteKing][PieceType::King];
        m_hash ^= zobrist::squarePieceKeys[m_blackKing][PieceType::King + blackOffset];
    }

}