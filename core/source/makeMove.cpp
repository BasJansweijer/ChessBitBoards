#include "bitBoard.h"
#include "chess.h"
#include <iostream>
#include "zobristHash.h"

namespace chess
{

    template <bool whitesMove>
    void BoardState::makePawnMove(const Move &move, square prevEnpassentLocation)
    {
        movePiece<Pawn, whitesMove>(move.from, move.to);

        constexpr uint8_t moveDir = whitesMove ? 1 : -1;

        if (prevEnpassentLocation == move.to)
            // Take the pawn of the opponent if we took via enpassent
            togglePiece<Pawn, !whitesMove>(move.to + 8 * -moveDir);

        // handle the update of the enpassent bitboard (have we moved two ranks)
        if (abs(move.to - move.from) == 2 * 8)
            m_enpassentSquare = move.from + moveDir * 8;
    }

    template <bool whitesMove>
    void BoardState::makeCastlingMove(const Move &move)
    {
        bool shortCastle = (move.to - move.from) == 2;
        bitboard *pieces = whitesMove ? m_whitePieces : m_blackPieces;
        square kingPos = whitesMove ? m_whiteKing : m_blackKing;

        square newKingPos;
        square oldRookPos;
        square newRookPos;

        if (shortCastle)
        {
            if constexpr (whitesMove)
            {
                newKingPos = 6;
                oldRookPos = 7;
                newRookPos = 5;
            }
            else
            {
                newKingPos = 6 + 7 * 8;
                oldRookPos = 7 + 7 * 8;
                newRookPos = 5 + 7 * 8;
            }
        }
        else
        {
            if constexpr (whitesMove)
            {
                newKingPos = 2;
                oldRookPos = 0;
                newRookPos = 3;
            }
            else
            {
                newKingPos = 2 + 7 * 8;
                oldRookPos = 0 + 7 * 8;
                newRookPos = 3 + 7 * 8;
            }
        }

        movePiece<Rook, whitesMove>(oldRookPos, newRookPos);
        movePiece<King, whitesMove>(kingPos, newKingPos);
    }

    template <bool whitesMove>
    void BoardState::makeKingMove(const Move &move)
    {
        // Check wether the move was castling
        if (abs(move.to - move.from) == 2)
        {
            makeCastlingMove<whitesMove>(move);
            return;
        }

        movePiece<King, whitesMove>(move.from, move.to);
    }

    void BoardState::updateCastelingRights(const Move &move)
    {
        if (move.piece == Rook)
        { // if we move a rook from its starting position we revoke the corresponding rights.
            switch (move.from)
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
        else if (move.piece == King)
        { // if we move the king we revoke castling rights
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
        }

        if (move.takesPiece)
        { // if there was a rook on that square we would have captured it so we revoke the rights
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
    }

    template <bool whitesMove>
    void BoardState::makePromotionMove(const Move &move)
    {
        // This is a promotion so we also need to remove the original pawn
        // appart from this we handle it as if it is a normal move by the promoted piece.
        togglePiece<Pawn, whitesMove>(move.from);

        // Togle the promoted piece.
        switch (move.piece)
        {
        case Knight:
            togglePiece<Knight, whitesMove>(move.to);
            break;
        case Bishop:
            togglePiece<Bishop, whitesMove>(move.to);
            break;
        case Rook:
            togglePiece<Rook, whitesMove>(move.to);
            break;
        case Queen:
            togglePiece<Queen, whitesMove>(move.to);
            break;
        default:
            throw std::runtime_error("Unsupported promotion piece type");
        }
    }

    template <bool whitesMove>
    void BoardState::makeMove(const Move &move)
    {
        // Remove the old enpassent location info
        square prevEnpassentLoc = m_enpassentSquare;
        m_enpassentSquare = -1;

        if (move.takesPiece)
        {
            // remove the piece on the square we moved to
            PieceType takenPiece = pieceOnSquare<!whitesMove>(move.to);
            togglePiece<!whitesMove>(takenPiece, move.to);
        }

        if (move.promotion)
            makePromotionMove<whitesMove>(move);
        else
        {
            // non promotion moves
            switch (move.piece)
            {
            case Pawn:
                makePawnMove<whitesMove>(move, prevEnpassentLoc);
                break;
            case Knight:
                movePiece<Knight, whitesMove>(move.from, move.to);
                break;
            case Bishop:
                movePiece<Bishop, whitesMove>(move.from, move.to);
                break;
            case Rook:
                movePiece<Rook, whitesMove>(move.from, move.to);
                break;
            case Queen:
                movePiece<Queen, whitesMove>(move.from, move.to);
                break;
            case King:
                makeKingMove<whitesMove>(move);
                break;
            default:
                throw std::runtime_error("Unknown piece type in makeMove!");
            }
        }

        updateCastelingRights(move);
    }

    template void BoardState::makeMove<true>(const Move &move);
    template void BoardState::makeMove<false>(const Move &move);

    void BoardState::makeMove(const Move &move)
    {

        // toggle old enpassent in hash
        // m_hash ^= zobrist::getEnpassentKey(m_enpassentSquare);

        // toggle old castling key
        // m_hash ^= zobrist::getCastlingKey(m_whiteCanCastleShort, m_whiteCanCastleLong,
        //                                   m_blackCanCastleShort, m_blackCanCastleLong);

        m_whitesMove ? makeMove<true>(move) : makeMove<false>(move);

        // Give the turn to the other player
        m_whitesMove = !m_whitesMove;

        // toggle turn key in hash
        // m_hash ^= zobrist::turnKey;

        // // toggle enpassent square in hash
        // m_hash ^= zobrist::getEnpassentKey(m_enpassentSquare);

        // // toggle on the updated castling rights:
        // m_hash ^= zobrist::getCastlingKey(m_whiteCanCastleShort, m_whiteCanCastleLong,
        //                                   m_blackCanCastleShort, m_blackCanCastleLong);
    }
}