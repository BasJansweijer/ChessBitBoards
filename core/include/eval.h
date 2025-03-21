#pragma once

#include "chess.h"
#include "limits.h"
#include <iostream>

namespace chess
{

    // The minimum int that is still forced mate
    constexpr int MATE_EVAL = 2000000000;

    class Eval
    {
    public:
        enum Type
        {
            SCORE,
            MATE
        } type;

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
                if (eval.mateIn >= 0)
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