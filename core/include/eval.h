#pragma once

#include "chess.h"
#include "limits.h"
#include <iostream>

#include "types.h"

namespace chess
{

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

    score evaluate(BoardState b);

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