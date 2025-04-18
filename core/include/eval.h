#pragma once

#include "chess.h"
#include "limits.h"
#include <iostream>

#include "types.h"

namespace chess
{
    constexpr int pieceVals[5] = {100, 300, 320, 500, 900};
    // The initial value of all (non pawn) pieces for a single player
    using PT = chess::PieceType;
    constexpr int startingPieceMaterial = pieceVals[PT::Knight] * 2 +
                                          pieceVals[PT::Bishop] * 2 +
                                          pieceVals[PT::Rook] * 2 +
                                          pieceVals[PT::Queen];

    enum class FileType : uint8_t
    {
        CLOSED = 0,
        HALF_OPEN_WHITE = 0b1,                   // no white pawn
        HALF_OPEN_BLACK = 0b10,                  // no black pawn
        OPEN = HALF_OPEN_BLACK | HALF_OPEN_WHITE // no pawns at all
    };

    // A class used to encapsulate all the data used during the evaluation process
    class Evaluator
    {
    public:
        // The main way to evaluate a position
        static score evaluate(const BoardState &b)
        {
            Evaluator evaluator(b);
            return evaluator.evaluation();
        }

        Evaluator() = delete;
        Evaluator(const BoardState &position)
            : m_board(position)
        {
            // Ensure all variables stored in the class are initialized

            // Set the piece sets
            m_whiteBitBoards = m_board.getPieceSet(true);
            m_blackBitBoards = m_board.getPieceSet(false);

            // Set the material / piece counts
            calculateMaterial();

            // set endGameNessScore
            calculateEndGameNess();

            // set middleGameScore and endGameScore
            calculatePieceSquareTableScores();

            // set fileTypes
            determineOpenFiles();
        }

        score evaluation();

        inline score getMaterialBalance() const
        {
            // material including pawns:
            score whiteMaterial = m_whitePieceMaterial + m_whitePieceCounts[Pawn] * pieceVals[Pawn];
            score blackMaterial = m_blackPieceMaterial + m_blackPieceCounts[Pawn] * pieceVals[Pawn];
            return whiteMaterial - blackMaterial;
        }

    private:
        void determineOpenFiles();
        float mopUpFactor(); // [0, 1] wether to use mopup score
        score mopUpScore();

        void calculateMaterial();
        void calculateEndGameNess();
        void calculatePieceSquareTableScores();

        template <bool isWhite>
        score kingSafety();

        template <bool isWhite>
        score rookOpenFileBonus();

        template <bool isWhite>
        score pawnStructureAnalysis();

        template <bool isWhite>
        score kingPositionScore();

        template <bool isWhite>
        score bishopPairBonus();

    private:
        const BoardState &m_board;

        const bitboard *m_whiteBitBoards;
        const bitboard *m_blackBitBoards;

        uint8_t m_whitePieceCounts[5];
        uint8_t m_blackPieceCounts[5];
        score m_whitePieceMaterial;
        score m_blackPieceMaterial;

        float m_endGameNessScore;   // [0, 1]
        float m_piecesMaterialLeft; // [0, 1]

        // Piece square table scores
        score m_middleGameScore;
        score m_endGameScore;

        FileType m_fileTypes[8];
    };

    // fileType helpers
    inline FileType &operator|=(FileType &lhs, FileType rhs)
    {
        lhs = (FileType)((uint8_t)lhs | (uint8_t)rhs);
        return lhs;
    }

    template <bool isWhite>
    inline bool containsPawn(FileType ft)
    {
        constexpr uint8_t ftMask = isWhite
                                       ? (uint8_t)FileType::HALF_OPEN_WHITE
                                       : (uint8_t)FileType::HALF_OPEN_BLACK;
        // Not half open for white means white pawn
        return !((uint8_t)ft & ftMask);
    }

    // A conveinient class to store the evaluation of a position (outside of search)
    class Eval
    {
    public:
        enum Type
        {
            SCORE,
            MATE
        } type;

        Eval(Type t, score n) : type(t)
        {
            if (t == MATE)
                mateIn = n;
            else
                scoreVal = n;
        }

        int16_t movesTillMate() const
        {
            if (type == MATE)
                return mateIn;
            else
                throw std::runtime_error("Eval is not a mate");
        }

    private:
        union
        {
            score scoreVal;
            int16_t mateIn;
        };

        // Overload operator<< for printing
        friend std::ostream &operator<<(std::ostream &os, const Eval &eval)
        {
            if (eval.type == MATE)
            {
                if (eval.mateIn >= 0)
                    os << "+M" << eval.mateIn;
                else
                    os << "-M" << -eval.mateIn;
            }
            else
            {
                os << eval.scoreVal;
            }
            return os;
        }
    };

    // Returns the eval class which contains information on wether it is mate
    // in a more friendly format (not used during search)
    inline Eval evalFromScore(score score, int searchDepth)
    {
        // check if it is not a mate
        if (abs(score) < MIN_MATE_SCORE)
            return Eval(Eval::Type::SCORE, score);

        bool whiteMating = score > 0;

        // Search depth that was remaining when the mate was found
        int depth = MAX_MATE_SCORE - abs(score);

        int n = (depth + 1) / 2;

        // For black mates we do minus n
        return Eval(Eval::Type::MATE, whiteMating ? n : -n);
    }
}