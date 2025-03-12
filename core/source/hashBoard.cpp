#include "chess.h"
#include "transposition.h"

namespace chess
{
    constexpr uint32_t prime = 0x100000001b3;

    inline void addPieceSet(const BoardState::PieceSet &set, uint64_t &hash)
    {
        hash ^= set.pawns;
        hash *= prime;

        hash ^= set.knights;
        hash *= prime;

        hash ^= set.bishops;
        hash *= prime;

        hash ^= set.rooks;
        hash *= prime;

        hash ^= set.queens;
        hash *= prime;

        hash ^= set.king;
        hash *= prime;
    }

    uint64_t BoardState::hash() const
    {

        uint64_t hash = 0xcbf29ce484222325;

        addPieceSet(m_white, hash);
        addPieceSet(m_black, hash);

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