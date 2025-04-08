/*
This file contains the implementation of initializing the start position
*/

#include "bitBoard.h"
#include "chess.h"

namespace chess
{

    BoardState::BoardState()
    {
        // Setup board:
        m_whitePieces[PieceType::Pawn] = 0xFF00;
        m_whitePieces[PieceType::Knight] = 0b01000010;
        m_whitePieces[PieceType::Bishop] = 0b00100100;
        m_whitePieces[PieceType::Rook] = 0b10000001;
        m_whitePieces[PieceType::Queen] = 0b00001000;
        m_whiteKing = 4;

        m_blackPieces[PieceType::Pawn] = m_whitePieces[PieceType::Pawn] << 8 * 5;
        m_blackPieces[PieceType::Knight] = m_whitePieces[PieceType::Knight] << 8 * 7;
        m_blackPieces[PieceType::Bishop] = m_whitePieces[PieceType::Bishop] << 8 * 7;
        m_blackPieces[PieceType::Rook] = m_whitePieces[PieceType::Rook] << 8 * 7;
        m_blackPieces[PieceType::Queen] = m_whitePieces[PieceType::Queen] << 8 * 7;
        m_blackKing = m_whiteKing + 8 * 7;

        m_enpassentSquare = -1;
        m_castleRights = 0b1111;
        m_whitesMove = true;

        m_pliesSince50MoveRuleReset = 0;
        m_ply = 0;

        // Ensure an up to date hash
        recomputeHash();
    }
}