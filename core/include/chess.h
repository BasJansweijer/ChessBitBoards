#pragma once

#include "bitBoard.h"
#include <string>
#include <tuple>
#include <vector>

namespace chess
{
    class BoardState
    {
    public:
        // Starting position
        BoardState();

        // custom position
        BoardState(std::string_view fen);

        // Returns all of the reachable board states from the current board
        std::vector<BoardState> nextStates() const;

        // Getters for the bitboards
        bitboard getWhitePawns() const { return m_whitePawns; }
        bitboard getWhiteKnights() const { return m_whiteKnights; }
        bitboard getWhiteBishops() const { return m_whiteBishops; }
        bitboard getWhiteRooks() const { return m_whiteRooks; }
        bitboard getWhiteQueens() const { return m_whiteQueens; }
        bitboard getWhiteKing() const { return m_whiteKing; }
        bitboard getBlackPawns() const { return m_blackPawns; }
        bitboard getBlackKnights() const { return m_blackKnights; }
        bitboard getBlackBishops() const { return m_blackBishops; }
        bitboard getBlackRooks() const { return m_blackRooks; }
        bitboard getBlackQueens() const { return m_blackQueens; }
        bitboard getBlackKing() const { return m_blackKing; }
        bitboard getEnpassentLocations() const { return m_enpassentLocations; }

        bool canWhiteCastleShort() const { return m_whiteCanCastleShort; }
        bool CanWhiteCastleLong() const { return m_whiteCanCastleLong; }
        bool canBlackCastleShort() const { return m_blackCanCastleShort; }
        bool CanBlackCastleLong() const { return m_blackCanCastleLong; }

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

        bool m_whitesMove;
    };
}