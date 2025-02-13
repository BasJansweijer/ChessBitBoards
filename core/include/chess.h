#pragma once

#include "bitBoard.h"
#include <string>

namespace chess
{
    class BoardState
    {
    public:
        // Starting position
        BoardState();

        // custom position
        BoardState(std::string_view fen);

    private:
        // We need 12 bit boards (6 for each color)
        // color white
        bitboard m_whitePawns;
        bitboard m_whiteKnights;
        bitboard m_whiteBishops;
        bitboard m_whiteRooks;
        bitboard m_whiteQueens;
        bitboard m_whiteKing;
        // color black
        bitboard m_blackPawns;
        bitboard m_blackKnights;
        bitboard m_blackBishops;
        bitboard m_blackRooks;
        bitboard m_blackQueens;
        bitboard m_blackKing;

        // Tracking enpassant oppertunities
        //  (1 bit means the square acts as if it can be taken)
        bitboard m_enpassentLocations;

        // Tracking for castling
        bool m_whiteCanCastleLong;
        bool m_whiteCanCastleShort;
        bool m_blackCanCastleLong;
        bool m_blackCanCastleShort;
    };
}