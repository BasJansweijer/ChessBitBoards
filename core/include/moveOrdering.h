#pragma once
/*
 * This file contains all the definitions for the "moveTables" that are used to improve the move ordering.
 * This includes the following tables:
 *  - History table
 */

#include "chess.h"
#include <algorithm>

namespace chess
{
    constexpr uint16_t moveIdx(Move m)
    {
        // note that the NULL move gets index 0.
        // also note that PAWN to a1 is not a legal move in any position as
        // the piece will be the piece type that is promoted to.
        return m.to | (m.piece << 6);
    }

    class MoveScorer
    {
    public:
        // For the indexing we only use the piece type and to square as these are most important
        // for charachterizing the move and we want to avoid having too many indices.

        // The max id is for the piece type with the highest number (5) and the max to square (63)
        static constexpr int NUM_MOVES = 1 + moveIdx(Move(0, 63, (PieceType)5, false));

        static constexpr score TABLE_MAX = SCORE_MAX - 3000;

        MoveScorer()
        {
            memset(&m_historyTable[0], SCORE_MIN, sizeof(m_historyTable));
        }

        // Gives a rough heuristic based on which we can order the moves in our search
        // Put in this class because we can use info from the search to help the ordering
        score moveScore(Move move, const BoardState &board) const;

        void registerBetaCutOff(Move m, uint8_t remainingDepth)
        {
            uint16_t idx = moveIdx(m);
            score bonus = remainingDepth * remainingDepth;

            // update history table
            m_historyTable[idx] += bonus;
            m_historyTable[idx] = std::min(m_historyTable[idx], TABLE_MAX);
        }

        // Orders the moves in the move list
        // We also use the transposition table to get the best move
        // from the previous search and put it at the front of the list
        // additionally for countermove heuristic we also need the previous move
        void orderMoves(MoveList &moves, const BoardState &board, Move TTMove)
        {
            std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b)
                      {
                // Check if the move is the best move from the transposition table
                if (a == TTMove)
                return true;
                
                if (b == TTMove)
                return false;
                
                // If not, we sort the moves based on their score
                return moveScore(a, board) > moveScore(b, board); });
        }

    private:
        score m_historyTable[NUM_MOVES];
    };

}