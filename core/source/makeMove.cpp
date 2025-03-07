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
        // remove old position and place on new position
        effectedBitboard ^= 1ULL << move.to | 1ULL << move.from;

        if (move.takesPiece)
        {
            // Check wether we take a rook and should thus revoke castling rights
            if ((m_whitesMove ? m_black.rooks : m_white.rooks) & 1ULL << move.to)
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
        bitboard &pawns = m_whitesMove ? m_white.pawns : m_black.pawns;
        makeNormalMove(move, pawns);

        uint8_t moveDir = m_whitesMove ? 1 : -1;

        if (prevEnpassentLocation == move.to)
        {
            // Take the pawn if we took via enpassent
            square takenPawn = move.to + 8 * -moveDir;
            bitboard &oppPawns = m_whitesMove ? m_black.pawns : m_white.pawns;
            oppPawns ^= 1ULL << takenPawn;
        }

        // handle the update of the enpassent bitboard (have we moved two ranks)
        if (abs(move.to - move.from) == 2 * 8)
            m_enpassentSquare = move.from + moveDir * 8;
    }

    void BoardState::makeCastlingMove(const Move &move)
    {
        bool shortCastle = (move.to - move.from) == 2;
        PieceSet &pieces = m_whitesMove ? m_white : m_black;

        if (shortCastle)
        {
            pieces.king = 6;

            square oldRookPos = 7;
            square newRookPos = 5;
            if (!m_whitesMove)
            {
                pieces.king += 7 * 8;
                oldRookPos += 7 * 8;
                newRookPos += 7 * 8;
            }

            pieces.rooks ^= 1ULL << oldRookPos;
            pieces.rooks ^= 1ULL << newRookPos;
        }
        else
        {
            pieces.king = 2;

            square oldRookPos = 0;
            square newRookPos = 3;
            if (!m_whitesMove)
            {
                pieces.king += 7 * 8;
                oldRookPos += 7 * 8;
                newRookPos += 7 * 8;
            }

            pieces.rooks ^= 1ULL << oldRookPos;
            pieces.rooks ^= 1ULL << newRookPos;
        }
    }

    void BoardState::makeKingMove(const Move &move)
    {
        // Moving king so we revoke castling rights from here on out.
        if (m_whitesMove)
        {
            m_whiteCanCastleLong = false;
            m_whiteCanCastleShort = false;
        }
        else
        {
            m_blackCanCastleLong = false;
            m_blackCanCastleShort = false;
        }

        if (abs(move.to - move.from) == 2)
        {
            makeCastlingMove(move);
            return;
        }

        square &king = m_whitesMove ? m_white.king : m_black.king;
        bitboard newLoc = 1ULL << king;
        makeNormalMove(move, newLoc);
        king = chess::bitBoards::firstSetBit(newLoc);
    }

    void BoardState::makeMove(const Move &move)
    {
        // Remove the old enpassent location info
        square prevEnpassentLoc = m_enpassentSquare;
        m_enpassentSquare = -1;

        if (move.promotion)
        {
            // This is a promotion so we also need to remove the original pawn
            // appart from this we handle it as if it is a normal move by the promoted piece.
            bitboard &pawns = m_whitesMove ? m_white.pawns : m_black.pawns;
            pawns ^= 1ULL << move.from;
        }

        switch (move.piece)
        {
        case PieceType::Pawn:
            makePawnMove(move, prevEnpassentLoc);
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

            if (move.from == 7)
                m_whiteCanCastleShort = false;

            if (move.from == 0)
                m_whiteCanCastleLong = false;

            if (move.from == 63)
                m_blackCanCastleShort = false;

            if (move.from == 56)
                m_blackCanCastleLong = false;

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
            makeKingMove(move);
            break;
        }
        default:
            throw std::runtime_error("Piece not yet movable");
            break;
        }

        // Update the all pieces bitboards for black and white
        m_white.updateAllPieces();
        m_black.updateAllPieces();

        // Give the turn to the other player
        m_whitesMove = !m_whitesMove;
    }
}