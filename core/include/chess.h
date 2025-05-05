#pragma once

#include "types.h"
#include <string>
#include "zobristHash.h"
#include <cstring>

namespace chess
{

    struct Move // (4 bytes)
    {
        // stores the move locations
        square from;
        square to;

        // stores the moved piece (new piece type for promotion)
        PieceType piece;

        Move() = default;

        // Defines the Null move
        static Move Null()
        {
            return Move(0, 0, PieceType::Pawn, false);
        }

        bool isNull() const
        {
            return from == 0 && to == 0;
        }

        Move(square from, square to, PieceType piece, bool takesPiece)
            : from(from), to(to),
              piece(piece), m_flags(takesPiece ? 0b1 : 0)
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

            if (isPromotion())
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

        /*
         * Returns a unique 2 byte index that can be used in any
         * tables indexed by moves.
         * In actuality only the first 15 bytes are used (can be 0-32767)
         */
        static constexpr int MAX_MOVE_IDX = 32767;
        uint16_t uniqueIndex() const
        {
            uint16_t idx = to;
            idx |= from << 6;
            idx |= piece << 12;

            return idx;
        }

        bool resets50MoveRule() const
        {
            return isPromotion() || isCapture() || piece == PieceType::Pawn;
        }

        inline bool isCapture() const
        {
            return m_flags & 0b1;
        }

        inline void setPromotion()
        {
            m_flags |= 0b10;
        }

        inline bool isPromotion() const
        {
            return m_flags & 0b10;
        }

        inline bool operator==(const Move &other) const
        {
            // we simply compare the memory of both move objects
            return std::memcmp(this, &other, sizeof(Move)) == 0;
        }

    private:
        /*
         * bit 1: is capture
         * bit 2: is promotion
         */
        uint8_t m_flags;
    };

    // We assume no possition has more possible moves than this.
    constexpr int MAX_MOVES = 256;

    class MoveList
    {
    public:
        uint8_t numMoves = 0;

        MoveList() = default;

        int size() { return numMoves; }
        Move *end() { return &moves[numMoves]; }
        Move *begin() { return moves; }

        Move &operator[](int index) { return moves[index]; }

        void emplace_back(square from, square to, PieceType piece, bool takesPiece)
        {
            new (&moves[numMoves]) Move(from, to, piece, takesPiece);
            numMoves++;
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

        enum MoveGenType
        {
            Normal,
            Quiescent
        };

        /**
         * @brief returns pseudo legal moves from the current position
         *
         * Pseudo legal means we don't consider wether we put ourselfs in check
         *
         * @return the moves
         */
        template <MoveGenType GenType>
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
        square getWhiteKingSquare() const { return m_whiteKing; }
        bitboard getBlackPawns() const { return m_blackPieces[PieceType::Pawn]; }
        bitboard getBlackKnights() const { return m_blackPieces[PieceType::Knight]; }
        bitboard getBlackBishops() const { return m_blackPieces[PieceType::Bishop]; }
        bitboard getBlackRooks() const { return m_blackPieces[PieceType::Rook]; }
        bitboard getBlackQueens() const { return m_blackPieces[PieceType::Queen]; }
        bitboard getBlackKing() const { return 1ULL << m_blackKing; }
        square getBlackKingSquare() const { return m_blackKing; }
        bitboard getEnpassentLocations() const { return 1ULL << m_enpassentSquare; }

        inline bool whiteCanCastleShort() const { return m_castleRights & 0b1; }
        inline bool whiteCanCastleLong() const { return m_castleRights & 0b10; }
        inline bool blackCanCastleShort() const { return m_castleRights & 0b100; }
        inline bool blackCanCastleLong() const { return m_castleRights & 0b1000; }

        inline bool drawBy50MoveRule() const { return m_pliesSince50MoveRuleReset >= 100; }

        inline uint8_t pliesTill50MoveRule() const { return 100 - m_pliesSince50MoveRuleReset; }

        inline uint16_t ply() const { return m_ply; }

        // Returns wether white/black has a piece attacking square s.
        template <bool ByWhite>
        bool squareAttacked(square s) const;
        bool kingAttacked(bool white) const;

        inline bool whitesMove() const { return m_whitesMove; }

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
        inline key getHash() const { return m_hash; }

        // Used only to get the hash of the current position to store in the repitition table (not the other meta data)
        inline key repetitionHash() const
        {
            return m_hash ^ zobrist::getEnpassentKey(m_enpassentSquare) ^ zobrist::get50MoveRuleKey(m_pliesSince50MoveRuleReset);
        }

        // Usually the hash is kept up to date, but in some cases (initialization mainly) we need to compute
        // the up to date hash
        // NOTE: this should not be used outside of testing purposes
        void recomputeHash();

        template <bool white>
        PieceType pieceOnSquare(square s) const;

    private:
        bitboard m_whitePieces[5];
        bitboard m_blackPieces[5];

        square m_whiteKing;
        square m_blackKing;

        // Tracking enpassant oppertunities
        //  (1 bit means the square acts as if it can be taken)
        square m_enpassentSquare;

        // Tracking for castling
        // bit 1: white short
        // bit 2: white long
        // bit 3: black short
        // bit 4: black long
        uint8_t m_castleRights;

        bool m_whitesMove;

        uint8_t m_pliesSince50MoveRuleReset;
        uint16_t m_ply;

        // Zobrist hash of the current board state
        key m_hash;

    private:
        // Piece specific move generation helpers
        template <MoveGenType GenT>
        void genPawnMoves(MoveList &outMoves) const;
        template <MoveGenType GenT>
        void genKnightMoves(MoveList &outMoves) const;
        template <MoveGenType GenT>
        void genBishopMoves(MoveList &outMoves) const;
        template <MoveGenType GenT>
        void genRookMoves(MoveList &outMoves) const;
        template <MoveGenType GenT>
        void genQueenMoves(MoveList &outMoves) const;
        template <MoveGenType GenT>
        void genKingMoves(MoveList &outMoves) const;

        // helpers to generate the castling moves
        void genCastlingMoves(MoveList &outMoves) const;
        void tryCastle(MoveList &outMoves, bool shortCastle) const;

        // helper which adds all moves positions specified in a bitboard
        template <MoveGenType GenT>
        inline void addMoves(bitboard moves, square curPos, PieceType piece, MoveList &outMoves) const;

        // makeMove templated
        template <bool whitesMove>
        void makeMove(const Move &move);

        // Piece specific move making helpers
        template <bool whitesMove>
        void makePawnMove(const Move &move, square prevEnpassentLocation);
        template <bool whitesMove>
        void makeKingMove(const Move &move); // Needed to also handle castling
        template <bool whitesMove>
        void makeCastlingMove(const Move &move);

        void updateCastelingRights(const Move &move);
        template <bool whitesMove>
        void makePromotionMove(const Move &move);

        // Helpers to move/remove pieces. (these methods also update the hash)
        template <PieceType piece, bool white>
        void movePiece(square from, square to);
        template <PieceType piece, bool white>
        void togglePiece(square s);
        template <bool white> // for cases where the piece is known not at compile time
        void togglePiece(PieceType piece, square s);

        // Helper which returns a char representing the piece (according to fen) int 0 if no piece is there
        char charOnSquare(square s) const;
    };
}