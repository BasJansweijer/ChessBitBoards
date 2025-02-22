/*
This file contains the implementation of both initializing the start position and initializing
from a given fen position.
*/

#include "bitBoard.h"
#include "chess.h"
#include "boardVisualizer.h"

namespace chess
{

    BoardState::BoardState()
    {
        // Setup board:
        m_white.pawns = 0xFF00;
        m_white.knights = 0b01000010;
        m_white.bishops = 0b00100100;
        m_white.rooks = 0b10000001;
        m_white.king = 0b00010000;
        m_white.queens = 0b00001000;

        m_black.pawns = m_white.pawns << 8 * 5;
        m_black.knights = m_white.knights << 8 * 7;
        m_black.bishops = m_white.bishops << 8 * 7;
        m_black.rooks = m_white.rooks << 8 * 7;
        m_black.queens = m_white.queens << 8 * 7;
        m_black.king = m_white.king << 8 * 7;

        m_enpassentSquare = -1;
        m_whiteCanCastleLong = true;
        m_whiteCanCastleShort = true;
        m_blackCanCastleLong = true;
        m_blackCanCastleShort = true;
        m_whitesMove = true;
    }
}