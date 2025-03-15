#pragma once

#include "chess.h"
#include "limits.h"
#include <iostream>

namespace chess
{

    constexpr int MAX_DEPTH = 250;
    // The minimum int that is still forced mate
    constexpr int MATE_EVAL = INT_MAX - MAX_DEPTH;

    class Eval
    {
    public:
        enum Type
        {
            SCORE,
            MATE
        } type;

        static Eval evalFromScore(int score)
        {
            // check if it is not a mate
            if (abs(score) < MATE_EVAL)
                return Eval(Eval::Type::SCORE, score);

            int mateInPlies = (score < 0 ? INT_MIN + score : INT_MAX - score);
            return Eval(Eval::Type::MATE, mateInPlies / 2);
        }

        Eval(Type t, int n) : type(t)
        {
            if (t == MATE)
                mateIn = n;
            else
                score = n;
        }

    private:
        union
        {
            int score;
            int mateIn;
        };

        // Overload operator<< for printing
        friend std::ostream &operator<<(std::ostream &os, const Eval &eval)
        {
            if (eval.type == MATE)
            {
                if (eval.mateIn > 0)
                    os << "+M" << eval.mateIn;
                else
                    os << "-M" << -eval.mateIn;
            }
            else
            {
                os << eval.score;
            }
            return os;
        }
    };

    // Returns the eval class which contains information on wether it is mate
    // in a more friendly format (not used during search)

    int evaluate(BoardState b);
}