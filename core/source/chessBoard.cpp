#include "bitBoard.h"
#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>

namespace chess
{

    BoardState::BoardState()
    {
        // Setup board:
        m_whitePawns = 0xFF00;
        m_whiteKnights = 0b01000010;
        m_whiteBishops = 0b00100100;
        m_whiteRooks = 0b10000001;
        m_whiteKing = 0b00010000;
        m_whiteQueens = 0b00001000;

        m_blackPawns = m_whitePawns << 8 * 5;
        m_blackKnights = m_whiteKnights << 8 * 7;
        m_blackBishops = m_whiteBishops << 8 * 7;
        m_blackRooks = m_whiteRooks << 8 * 7;
        m_blackQueens = m_whiteQueens << 8 * 7;
        m_blackKing = m_whiteKing << 8 * 7;

        m_enpassentLocations = 0;
        m_whiteCanCastleLong = true;
        m_whiteCanCastleShort = true;
        m_blackCanCastleLong = true;
        m_blackCanCastleShort = true;

        chess::bitBoards::showBitboardGUI(m_whitePawns);
    }

    BoardState::BoardState(std::string_view fen)
    {
        throw std::runtime_error("Not Implemented");
    }
}