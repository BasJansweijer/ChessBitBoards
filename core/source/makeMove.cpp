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
            m_black.pawns &= mask;
            m_black.knights &= mask;
            m_black.bishops &= mask;
            m_black.rooks &= mask;
            m_black.queens &= mask;
        }
        else
        {
            m_white.pawns &= mask;
            m_white.knights &= mask;
            m_white.bishops &= mask;
            m_white.rooks &= mask;
            m_white.queens &= mask;
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
        bitboard &pawns = m_whitesMove ? m_white.pawns : m_black.pawns;
        makeNormalMove(move, pawns);

        int moveDir = m_whitesMove ? 1 : -1;

        if (m_enpassentSquare == move.to)
        {
            square takenPawn = move.to + 8 * -moveDir;
            bitboard &oppPawns = m_whitesMove ? m_black.pawns : m_white.pawns;
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

        bitboard &pawns = m_whitesMove ? m_white.pawns : m_black.pawns;
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
            bitboard &knights = m_whitesMove ? m_white.knights : m_black.knights;
            makeNormalMove(move, knights);
            break;
        }
        case PieceType::Bishop:
        {
            bitboard &bishops = m_whitesMove ? m_white.bishops : m_black.bishops;
            makeNormalMove(move, bishops);
            break;
        }
        case PieceType::Rook:
        {
            bitboard &rooks = m_whitesMove ? m_white.rooks : m_black.rooks;
            makeNormalMove(move, rooks);
            break;
        }
        case PieceType::Queen:
        {
            bitboard &queens = m_whitesMove ? m_white.queens : m_black.queens;
            makeNormalMove(move, queens);
            break;
        }
        case PieceType::King:
        {
            bitboard &king = m_whitesMove ? m_white.king : m_black.king;
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