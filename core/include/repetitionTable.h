#pragma once

#include "chess.h"
#include "types.h"

namespace chess
{

    struct RepetitionTable
    {

        RepetitionTable()
        {
            m_length = 0;
        }

        // Not performance critical, this is only called when a move is made on the root board
        // via the engine makeMove method
        inline void addState(const BoardState &b)
        {
            if (!m_length)
                m_startsOnWhite = b.whitesMove();

            m_table[m_length] = b.hashWithoutEnpassent();
            m_length++;
        }

        inline void clear()
        {
            m_length = 0;
        }

        inline bool drawDueTo50MoveRule() const
        {
            return m_length >= 100;
        }

        /*
         * checks wether the repetition table contains the given board.
         * Note that we only check half of the boards as we can skip any
         * where it isn't the same colors turn.
         */
        inline bool contains(const BoardState &b) const
        {
            bool checkEvenIdx = b.whitesMove() == m_startsOnWhite;
            int idx = m_length - 1;

            // Ensure we start on an even/odd idx
            // (if checkEvenIdx is 1 and idx % 2 is 1 decrement 1)
            // (if checkEvenIdx is 0 and idx % 2 is 0 decrement 1)
            // (else do nothing)
            idx -= idx % 2 == checkEvenIdx;

            while (idx >= 0)
            {
                // If the b.getHash contains enpassent data then we cannot have repetition anyways!
                // (from the root we cannot do a double pawn move and have repetition at any point)
                if (m_table[idx] == b.getHash())
                    return true;

                // Skip hashes where it isn't the same colors turn
                idx -= 2;
            }

            return false;
        }

    private:
        uint8_t m_length;
        key m_table[100];

        // Tracks wether the first element is a board on which it is whites turn
        bool m_startsOnWhite;
    };
}