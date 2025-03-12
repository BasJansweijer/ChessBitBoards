#include <limits.h>
#include <algorithm>
#include <functional>

#include "chess.h"
#include "search.h"
#include <chrono>
#include <thread>

namespace chess
{
    void Search::startTimeThread(double thinkSeconds)
    {
        m_stopped = false; // Reset before starting

        // Launch a detached thread to stop search after thinkSeconds
        std::thread([this, thinkSeconds]()
                    {
            std::this_thread::sleep_for(std::chrono::duration<double>(thinkSeconds));
            m_stopped = true; })
            .detach();
    }

    std::tuple<Move, int, int> Search::iterativeDeepening(double thinkSeconds)
    {
        startTimeThread(thinkSeconds);

        Move bestMove;
        int eval;

        Move currentSearchBest;
        int prevSearchEval;

        // Initially only use depth of 2 (after the first increment)
        int depth = 1;

        while (!m_stopped)
        {
            // only update with each completed search
            eval = prevSearchEval;
            depth += 1;

            if (m_rootBoard.whitesMove())
                prevSearchEval = minimax<true, true>(m_rootBoard, depth, currentSearchBest);
            else
                prevSearchEval = minimax<false, true>(m_rootBoard, depth, currentSearchBest);

            bestMove = currentSearchBest;
        }

        return {bestMove, eval, depth};
    }

    template <bool Max, bool Root>
    int Search::minimax(BoardState &curBoard, int depth, Move &outMove, int alpha, int beta)
    {
        // cancel the search
        if (m_stopped.load(std::memory_order_relaxed))
            return 0;

        // Base case
        if (depth == 0)
            return m_evalFunc(curBoard);

        // Start with the worst possible eval
        int bestEval = Max ? INT_MIN : INT_MAX;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves();

        for (const Move &m : pseudoLegalMoves)
        {
            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(Max))
                continue; // skip since move was illegal

            // search with opposite of min/max and not root and 1 less depth
            int moveEval = minimax<!Max, false>(newBoard, depth - 1, outMove, alpha, beta);
            bestEval = Max ? std::max(bestEval, moveEval) : std::min(bestEval, moveEval);

            if (Max ? bestEval > beta : bestEval < alpha)
                break; // The opponent could have chosen a better move in a previous step.

            // max alpha / min beta depending on what player we are
            Max ? alpha = std::max(alpha, bestEval) : beta = std::min(beta, bestEval);

            // Incase this is the root search node then update if we find a better move.
            if (Root && moveEval == bestEval)
                outMove = m;
        }

        return bestEval;
    }

    template int Search::minimax<true, true>(BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, true>(BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<true, false>(BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, false>(BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
}