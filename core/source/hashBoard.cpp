#include "chess.h"
#include "transposition.h"

namespace chess
{
    constexpr uint32_t prime = 0x100000001b3;

    inline void addPieceSet(const bitboard *set, uint64_t &hash)
    {
        hash ^= set[PieceType::Pawn];
        hash *= prime;

        hash ^= set[PieceType::Knight];
        hash *= prime;

        hash ^= set[PieceType::Bishop];
        hash *= prime;

        hash ^= set[PieceType::Rook];
        hash *= prime;

        hash ^= set[PieceType::Queen];
        hash *= prime;
    }

    uint64_t BoardState::hash() const
    {

        uint64_t hash = 0xcbf29ce484222325;

        addPieceSet(m_whitePieces, hash);
        hash ^= m_whiteKing;
        hash *= prime;

        addPieceSet(m_blackPieces, hash);
        hash ^= m_blackKing;
        hash *= prime;

        uint8_t flags = 0;
        flags |= m_whiteCanCastleShort;
        flags |= m_whiteCanCastleLong << 1;
        flags |= m_blackCanCastleShort << 2;
        flags |= m_blackCanCastleLong << 3;
        flags |= m_whitesMove << 4;
        hash ^= flags;
        hash *= prime;

        return hash;
    }

}