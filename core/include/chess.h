#pragma once

#include "bitBoard.h"
#include <string>
#include <vector>

namespace chess
{
    enum PieceType
    {
        None = 0,
        Pawn,
        Knight,
        Bishop,
        Rook,
        King,
        Queen
    };

    struct Move
    {
        // stores the move locations
        square from;
        square to;

        // stores the moved piece (new piece type for promotion)
        PieceType piece;

        // stores what piece was taken
        bool takesPiece;

        Move(square from, square to, PieceType piece, bool takesPiece)
            : from(from), to(to),
              piece(piece), takesPiece(takesPiece) {}
    };

    class BoardState
    {
    public:
        // Starting position
        BoardState();

        // custom position
        BoardState(std::string_view fen);

        /**
         * @brief returns pseudo legal moves from the current position
         *
         * Pseudo legal means we don't consider wether we put ourselfs in check
         *
         * @return the moves
         */
        std::vector<Move> pseudoLegalMoves() const;

        /**
         * @brief returns the legal moves in the position
         *
         * This performs each pseudo legal move and checks wether it puts us in check.
         *
         * @return the moves
         */
        std::vector<Move> legalMoves() const;

        void makeMove(const Move &move);

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
        bitboard getEnpassentLocations() const { return 1ULL << m_enpassentSquare; }

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
        square m_enpassentSquare;

        // Tracking for castling
        bool m_whiteCanCastleLong;
        bool m_whiteCanCastleShort;
        bool m_blackCanCastleLong;
        bool m_blackCanCastleShort;

        bool m_whitesMove;

    private:
        // Piece specific move generation helpers
        void genPawnMoves(std::vector<Move> &outMoves) const;
        void genKnightMoves(std::vector<Move> &outMoves) const;
        void genKingMoves(std::vector<Move> &outMoves) const;

        // helper which adds all moves positions specified in a bitboard
        inline void addMoves(bitboard moves, square curPos, PieceType piece, std::vector<Move> &outMoves) const;

        bitboard allPieces(bool white) const;
        bitboard allPieces() const;

        // default move making implementation
        void makeNormalMove(const Move &move, bitboard &effectedBitboard);

        // Piece specific move making helpers
        void makePawnMove(const Move &move);

        // Makes the implicit assumption that the 'opponent' is the player whose turn it is NOT.
        void takeOpponentPiece(square s);
    };
}