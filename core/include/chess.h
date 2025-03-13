#pragma once

#include "bitBoard.h"
#include <string>

namespace chess
{
    enum PieceType
    {
        Pawn = 0,
        Knight,
        Bishop,
        Rook,
        Queen,
        King,
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
        bool promotion;

        Move() = default;

        Move(square from, square to, PieceType piece, bool takesPiece)
            : from(from), to(to),
              piece(piece), takesPiece(takesPiece), promotion(false)
        {
        }

        std::string toUCI() const
        {
            // Allocate a string large enough to hold the 4 characters (e.g., "e2e4")
            std::string uciMove(4, ' ');

            // Calculate the rank and file for the 'from' square
            uciMove[0] = from % 8 + 'a'; // File (a-h)
            uciMove[1] = from / 8 + '1'; // Rank (1-8)

            // Calculate the rank and file for the 'to' square
            uciMove[2] = to % 8 + 'a'; // File (a-h)
            uciMove[3] = to / 8 + '1'; // Rank (1-8)

            if (promotion)
            {
                switch (piece)
                {
                case PieceType::Knight:
                    uciMove += 'n';
                    break;
                case PieceType::Bishop:
                    uciMove += 'b';
                    break;
                case PieceType::Rook:
                    uciMove += 'r';
                    break;
                case PieceType::Queen:
                    uciMove += 'q';
                    break;
                }
            }

            return uciMove;
        }
    };

    // We assume no possition has more possible moves than this.
    constexpr int MAX_MOVES = 256;

    class MoveList
    {
    public:
        uint8_t numMoves = 0;

        MoveList() = default;

        int size() { return numMoves; }
        const Move *end() const { return &moves[numMoves]; }
        const Move *begin() const { return moves; }

        Move &operator[](int index) { return moves[index]; }

        void emplace_back(square from, square to, PieceType piece, bool takesPiece)
        {
            new (&moves[numMoves]) Move(from, to, piece, takesPiece);
            ++numMoves;
        }

        Move &back() { return moves[numMoves - 1]; }

        void push_back(const Move &m)
        {
            moves[numMoves] = m;
            numMoves++;
        }

    private:
        Move moves[MAX_MOVES];
    };

    class BoardState
    {
    public:
        // Starting position
        BoardState();

        // custom position
        BoardState(std::string_view fen);

        // Retrieves the fen of the current position
        std::string fen() const;

        /**
         * @brief returns pseudo legal moves from the current position
         *
         * Pseudo legal means we don't consider wether we put ourselfs in check
         *
         * @return the moves
         */
        MoveList pseudoLegalMoves() const;

        /**
         * @brief returns the legal moves in the position
         *
         * This performs each pseudo legal move and checks wether it puts us in check.
         *
         * @return the moves
         */
        MoveList legalMoves() const;

        void makeMove(const Move &move);

        const bitboard *getPieceSet(bool white) const { return white ? m_whitePieces : m_blackPieces; }
        inline bitboard allPieces(bool white) const { return white ? whitePieces() : blackPieces(); }
        inline bitboard allPieces() const { return whitePieces() | blackPieces(); }

        // Getters for the bitboards
        bitboard getWhitePawns() const { return m_whitePieces[PieceType::Pawn]; }
        bitboard getWhiteKnights() const { return m_whitePieces[PieceType::Knight]; }
        bitboard getWhiteBishops() const { return m_whitePieces[PieceType::Bishop]; }
        bitboard getWhiteRooks() const { return m_whitePieces[PieceType::Rook]; }
        bitboard getWhiteQueens() const { return m_whitePieces[PieceType::Queen]; }
        bitboard getWhiteKing() const { return 1ULL << m_whiteKing; }
        bitboard getBlackPawns() const { return m_blackPieces[PieceType::Pawn]; }
        bitboard getBlackKnights() const { return m_blackPieces[PieceType::Knight]; }
        bitboard getBlackBishops() const { return m_blackPieces[PieceType::Bishop]; }
        bitboard getBlackRooks() const { return m_blackPieces[PieceType::Rook]; }
        bitboard getBlackQueens() const { return m_blackPieces[PieceType::Queen]; }
        bitboard getBlackKing() const { return 1ULL << m_blackKing; }
        bitboard getEnpassentLocations() const { return 1ULL << m_enpassentSquare; }

        bool canWhiteCastleShort() const { return m_whiteCanCastleShort; }
        bool CanWhiteCastleLong() const { return m_whiteCanCastleLong; }
        bool canBlackCastleShort() const { return m_blackCanCastleShort; }
        bool CanBlackCastleLong() const { return m_blackCanCastleLong; }

        // Returns wether white/black has a piece attacking square s.
        template <bool ByWhite>
        bool squareAttacked(square s) const;
        bool kingAttacked(bool white) const;

        inline bool whitesMove() const { return m_whitesMove; }

        uint64_t hash() const;

    private:
        bitboard m_whitePieces[5];
        bitboard m_blackPieces[5];

        square m_whiteKing;
        square m_blackKing;

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
        void genPawnMoves(MoveList &outMoves) const;
        void genKnightMoves(MoveList &outMoves) const;
        void genBishopMoves(MoveList &outMoves) const;
        void genRookMoves(MoveList &outMoves) const;
        void genQueenMoves(MoveList &outMoves) const;
        void genKingMoves(MoveList &outMoves) const;

        // helpers to generate the castling moves
        void genCastlingMoves(MoveList &outMoves) const;
        void tryCastle(MoveList &outMoves, bool shortCastle) const;

        // helper which adds all moves positions specified in a bitboard
        inline void addMoves(bitboard moves, square curPos, PieceType piece, MoveList &outMoves) const;

        inline bitboard whitePieces() const
        {
            return m_whitePieces[PieceType::Pawn] | m_whitePieces[PieceType::Knight] |
                   m_whitePieces[PieceType::Bishop] | m_whitePieces[PieceType::Rook] |
                   m_whitePieces[PieceType::Queen] | 1ULL << m_whiteKing;
        }

        inline bitboard blackPieces() const
        {
            return m_blackPieces[PieceType::Pawn] | m_blackPieces[PieceType::Knight] |
                   m_blackPieces[PieceType::Bishop] | m_blackPieces[PieceType::Rook] |
                   m_blackPieces[PieceType::Queen] | 1ULL << m_blackKing;
        }

        // default move making implementation
        void makeNormalMove(const Move &move, bitboard &effectedBitboard);

        // Piece specific move making helpers
        void makePawnMove(const Move &move, square prevEnpassentLocation);
        void makeKingMove(const Move &move); // Needed to also handle castling
        void makeCastlingMove(const Move &move);

        // Makes the implicit assumption that the 'opponent' is the player whose turn it is NOT.
        void takeOpponentPiece(square s);

        // Helper which returns a char representing the piece (according to fen) int 0 if no piece is there
        char pieceOnSquare(square s) const;
    };
}