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
        bitboard getWhitePawns() const { return m_white.pawns; }
        bitboard getWhiteKnights() const { return m_white.knights; }
        bitboard getWhiteBishops() const { return m_white.bishops; }
        bitboard getWhiteRooks() const { return m_white.rooks; }
        bitboard getWhiteQueens() const { return m_white.queens; }
        bitboard getWhiteKing() const { return m_white.king; }
        bitboard getBlackPawns() const { return m_black.pawns; }
        bitboard getBlackKnights() const { return m_black.knights; }
        bitboard getBlackBishops() const { return m_black.bishops; }
        bitboard getBlackRooks() const { return m_black.rooks; }
        bitboard getBlackQueens() const { return m_black.queens; }
        bitboard getBlackKing() const { return m_black.king; }
        bitboard getEnpassentLocations() const { return 1ULL << m_enpassentSquare; }

        bool canWhiteCastleShort() const { return m_whiteCanCastleShort; }
        bool CanWhiteCastleLong() const { return m_whiteCanCastleLong; }
        bool canBlackCastleShort() const { return m_blackCanCastleShort; }
        bool CanBlackCastleLong() const { return m_blackCanCastleLong; }

        // Returns wether white/black has a piece attacking square s.
        bool squareAttacked(square s, bool byWhite) const;
        bool kingAttacked(bool white) const;

        bool whitesMove() const { return m_whitesMove; }

        // We need 12 bit boards (6 for each color)
        struct PieceSet
        {
            bitboard pawns, knights, bishops, rooks, queens, king;

            inline bitboard allPieces() const
            {
                return pawns | knights | bishops | rooks | queens | king;
            }
        };

    private:
        PieceSet m_white;
        PieceSet m_black;

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
        void genBishopMoves(std::vector<Move> &outMoves) const;
        void genRookMoves(std::vector<Move> &outMoves) const;
        void genQueenMoves(std::vector<Move> &outMoves) const;
        void genKingMoves(std::vector<Move> &outMoves) const;

        // helpers to generate the castling moves
        void genCastlingMoves(std::vector<Move> &outMoves) const;
        void tryCastle(std::vector<Move> &outMoves, bool shortCastle) const;

        // helper which adds all moves positions specified in a bitboard
        inline void addMoves(bitboard moves, square curPos, PieceType piece, std::vector<Move> &outMoves) const;

        bitboard allPieces(bool white) const;
        bitboard allPieces() const;

        // default move making implementation
        void makeNormalMove(const Move &move, bitboard &effectedBitboard);

        // Piece specific move making helpers
        void makePawnMove(const Move &move);
        void makeKingMove(const Move &move); // Needed to also handle castling
        void makeCastlingMove(const Move &move);

        // Makes the implicit assumption that the 'opponent' is the player whose turn it is NOT.
        void takeOpponentPiece(square s);
    };
}