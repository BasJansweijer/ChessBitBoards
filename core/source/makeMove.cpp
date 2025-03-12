#include "bitBoard.h"
#include "chess.h"
#include <iostream>

namespace chess
{
    void BoardState::takeOpponentPiece(square s)
    {
        // all non taken positions
        bitboard mask = ~(1ULL << s);
        if (m_whitesMove)
        {
            m_blackPieces[PieceType::Pawn] &= mask;
            m_blackPieces[PieceType::Knight] &= mask;
            m_blackPieces[PieceType::Bishop] &= mask;
            m_blackPieces[PieceType::Rook] &= mask;
            m_blackPieces[PieceType::Queen] &= mask;
        }
        else
        {
            m_whitePieces[PieceType::Pawn] &= mask;
            m_whitePieces[PieceType::Knight] &= mask;
            m_whitePieces[PieceType::Bishop] &= mask;
            m_whitePieces[PieceType::Rook] &= mask;
            m_whitePieces[PieceType::Queen] &= mask;
        }
    }

    void BoardState::makeNormalMove(const Move &move, bitboard &effectedBitboard)
    {
        // remove old position and place on new position
        // (If the move was a promotion we do not remove the original piece)
        move.promotion ? effectedBitboard ^= 1ULL << move.to
                       : effectedBitboard ^= 1ULL << move.to | 1ULL << move.from;

        if (move.takesPiece)
        {
            // Check wether we take a rook and should thus revoke castling rights
            if ((m_whitesMove ? m_blackPieces[PieceType::Rook] : m_whitePieces[PieceType::Rook]) & 1ULL << move.to)
            {
                switch (move.to)
                {
                case 63:
                    m_blackCanCastleShort = false;
                    break;
                case 56:
                    m_blackCanCastleLong = false;
                    break;
                case 0:
                    m_whiteCanCastleLong = false;
                    break;
                case 7:
                    m_whiteCanCastleShort = false;
                    break;
                }
            }

            takeOpponentPiece(move.to);
        }
    }

    void BoardState::makePawnMove(const Move &move, square prevEnpassentLocation)
    {
        bitboard &pawns = m_whitesMove ? m_whitePieces[PieceType::Pawn] : m_blackPieces[PieceType::Pawn];
        makeNormalMove(move, pawns);

        uint8_t moveDir = m_whitesMove ? 1 : -1;

        if (prevEnpassentLocation == move.to)
        {
            // Take the pawn if we took via enpassent
            square takenPawn = move.to + 8 * -moveDir;
            bitboard &oppPawns = m_whitesMove ? m_blackPieces[PieceType::Pawn] : m_whitePieces[PieceType::Pawn];
            oppPawns ^= 1ULL << takenPawn;
        }

        // handle the update of the enpassent bitboard (have we moved two ranks)
        if (abs(move.to - move.from) == 2 * 8)
            m_enpassentSquare = move.from + moveDir * 8;
    }

    void BoardState::makeCastlingMove(const Move &move)
    {
        bool shortCastle = (move.to - move.from) == 2;
        bitboard *pieces = m_whitesMove ? m_whitePieces : m_blackPieces;
        square &kingPos = m_whitesMove ? m_whiteKing : m_blackKing;

        square newKingPos;
        square oldRookPos;
        square newRookPos;

        if (shortCastle)
        {
            kingPos = 6;
            oldRookPos = 7;
            newRookPos = 5;
            if (!m_whitesMove)
            {
                kingPos += 7 * 8;
                oldRookPos += 7 * 8;
                newRookPos += 7 * 8;
            }
        }
        else
        {
            kingPos = 2;
            oldRookPos = 0;
            newRookPos = 3;
            if (!m_whitesMove)
            {
                kingPos += 7 * 8;
                oldRookPos += 7 * 8;
                newRookPos += 7 * 8;
            }
        }

        pieces[PieceType::Rook] ^= 1ULL << oldRookPos;
        pieces[PieceType::Rook] ^= 1ULL << newRookPos;
    }

    void BoardState::makeKingMove(const Move &move)
    {
        // Moving king so we revoke castling rights from here on out.
        if (m_whitesMove)
        {
            m_whiteCanCastleLong = false;
            m_whiteCanCastleShort = false;
            m_whiteKing = move.to;
        }
        else
        {
            m_blackCanCastleLong = false;
            m_blackCanCastleShort = false;
            m_blackKing = move.to;
        }

        if (abs(move.to - move.from) == 2)
        {
            makeCastlingMove(move);
            return;
        }

        bitboard temp;
        makeNormalMove(move, temp);
    }

    void BoardState::makeMove(const Move &move)
    {
        // Remove the old enpassent location info
        square prevEnpassentLoc = m_enpassentSquare;
        m_enpassentSquare = -1;

        bitboard *movingPieces = m_whitesMove ? m_whitePieces : m_blackPieces;

        if (move.promotion)
        {
            // This is a promotion so we also need to remove the original pawn
            // appart from this we handle it as if it is a normal move by the promoted piece.

            movingPieces[PieceType::Pawn] ^= 1ULL << move.from;
        }

        switch (move.piece)
        {
        case PieceType::Pawn:
            makePawnMove(move, prevEnpassentLoc);
            break;
        case PieceType::Rook:
        {
            if (move.from == 7)
                m_whiteCanCastleShort = false;

            if (move.from == 0)
                m_whiteCanCastleLong = false;

            if (move.from == 63)
                m_blackCanCastleShort = false;

            if (move.from == 56)
                m_blackCanCastleLong = false;

            makeNormalMove(move, movingPieces[PieceType::Rook]);
            break;
        }
        case PieceType::King:
        {
            makeKingMove(move);
            break;
        }
        default: // Queen, knights or bishops
            makeNormalMove(move, movingPieces[move.piece]);
            break;
        }

        // Give the turn to the other player
        m_whitesMove = !m_whitesMove;
        updateAllPiecesBB();
    }
}