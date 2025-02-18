#include "bitBoard.h"
#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>

namespace chess
{
    void BoardState::takeOpponentPiece(square s)
    {
        // all non taken positions
        bitboard mask = ~(1ULL << s);
        if (m_whitesMove)
        {
            m_blackPawns &= mask;
            m_blackKnights &= mask;
            m_blackBishops &= mask;
            m_blackRooks &= mask;
            m_blackQueens &= mask;
        }
        else
        {
            m_whitePawns &= mask;
            m_whiteKnights &= mask;
            m_whiteBishops &= mask;
            m_whiteRooks &= mask;
            m_whiteQueens &= mask;
        }
    }

    void BoardState::makeNormalMove(const Move &move, bitboard &effectedBitboard)
    {
        effectedBitboard &= ~(1ULL << move.from);
        effectedBitboard ^= 1ULL << move.to;

        if (move.takesPiece)
            takeOpponentPiece(move.to);
    }

    void BoardState::makePawnMove(const Move &move)
    {
        bitboard &pawns = m_whitesMove ? m_whitePawns : m_blackPawns;
        makeNormalMove(move, pawns);

        int moveDir = m_whitesMove ? 1 : -1;

        if (m_enpassentSquare == move.to)
        {
            square takenPawn = move.to + 8 * -moveDir;
            bitboard &oppPawns = m_whitesMove ? m_blackBishops : m_whitePawns;
            oppPawns ^= 1ULL << takenPawn;
        }

        // handle the update of the enpassent bitboard
        int movedRanks = abs(move.to - move.from) / 8;
        if (movedRanks == 2)
        {
            m_enpassentSquare = move.from + moveDir * 8;
        }
    }

    void BoardState::makeMove(const Move &move)
    {
        // Remove the old enpassent location info
        m_enpassentSquare = -1;

        bitboard &pawns = m_whitesMove ? m_whitePawns : m_blackPawns;
        if (pawns & 1ULL << move.from && move.piece != PieceType::Pawn)
        {
            // This is a promotion so we also need to remove the original pawn
            // appart from this we handle it as if it is a normal move by the promoted piece.
            pawns ^= 1ULL << move.from;
        }

        switch (move.piece)
        {
        case PieceType::Pawn:
            makePawnMove(move);
            break;
        case PieceType::Knight:
        {
            bitboard &knights = m_whitesMove ? m_whiteKnights : m_blackKnights;
            makeNormalMove(move, knights);
            break;
        }
        case PieceType::Bishop:
        {
            bitboard &bishops = m_whitesMove ? m_whiteBishops : m_blackBishops;
            makeNormalMove(move, bishops);
            break;
        }
        case PieceType::Rook:
        {
            bitboard &rooks = m_whitesMove ? m_whiteRooks : m_blackRooks;
            makeNormalMove(move, rooks);
            break;
        }
        case PieceType::Queen:
        {
            bitboard &queens = m_whitesMove ? m_whiteQueens : m_blackQueens;
            makeNormalMove(move, queens);
            break;
        }
        case PieceType::King:
        {
            bitboard &king = m_whitesMove ? m_whiteKing : m_blackKing;
            makeNormalMove(move, king);
            break;
        }
        default:
            throw std::runtime_error("Piece not yet movable");
            break;
        }

        // Give the turn to the other player
        m_whitesMove = !m_whitesMove;
    }
}