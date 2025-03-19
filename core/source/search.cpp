#include <limits.h>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>

#include "chess.h"
#include "search.h"

namespace chess
{
    void Search::startTimeThread(double thinkSeconds)
    {
        m_stopped = false; // Reset before starting
        m_cancelled = false;

        // Launch a detached thread to stop search after thinkSeconds
        m_timerThread = std::thread([this, thinkSeconds]()
                                    {
                                        auto endTime = std::chrono::steady_clock::now() + std::chrono::duration<double>(thinkSeconds);

                                        while (std::chrono::steady_clock::now() < endTime)
                                        {
                                            if (m_cancelled)
                                            {
                                                // If cancelled, exit early
                                                m_stopped = true;
                                                return;
                                            }

                                            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short time to check cancellation
                                        }
                                        m_stopped = true; // Time expired, mark as stopped
                                    });
    }

    void Search::stopTimeThread()
    {
        m_cancelled = true;
        m_timerThread.join();
    }

    std::tuple<Move, Eval, int> Search::iterativeDeepening(double thinkSeconds)
    {
        startTimeThread(thinkSeconds);

        Move bestMove = Move::Null();
        int evalScore;
        Eval eval = Eval::evalFromScore(0);

        Move currentSearchBest;
        int newScore;

        // Initially only use depth of 2 (after the first increment)
        int depth = 1;

        const bool root = true;

        while (eval.type != Eval::Type::MATE)
        {
            depth += 1;
            if (m_rootBoard.whitesMove())
                newScore = minimax<true, root>(m_rootBoard, depth, currentSearchBest);
            else
                newScore = minimax<false, root>(m_rootBoard, depth, currentSearchBest);

            // if search is stopped early return using the previous depth results
            if (m_stopped)
            {
                // highest completed depth is one less
                depth -= 1;
                break;
            }

            // only update with each completed search
            evalScore = newScore;
            eval = Eval::evalFromScore(evalScore);
            bestMove = currentSearchBest;
        }

        // The search is done so we stop any still going timer
        stopTimeThread();
        return {bestMove, eval, depth};
    }

    template <bool Max, bool Root>
    int Search::minimax(const BoardState &curBoard, int depth, Move &outMove, int alpha, int beta)
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

            // Incase this is the root search node then update if the eval was updated
            if (Root && moveEval == bestEval)
                outMove = m;
        }

        // Stalemate detection
        bool noLegalMoves = bestEval == INT_MIN || bestEval == INT_MAX;
        if (noLegalMoves && !curBoard.kingAttacked(curBoard.whitesMove()))
        {
            // Stalemate is a draw.
            return 0;
        }

        // If the eval is a mate in n then decrease/Increase the eval by one
        // to make the engine prefer quick mates
        bestEval += bestEval > MATE_EVAL      ? -1
                    : (bestEval < -MATE_EVAL) ? 1
                                              : 0;

        return bestEval;
    }

    template int Search::minimax<true, true>(const BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, true>(const BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<true, false>(const BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, false>(const BoardState &curBoard, int depth, Move &outMove, int alpha, int beta);
}