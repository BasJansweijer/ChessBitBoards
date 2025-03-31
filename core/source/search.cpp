#include <limits.h>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <cmath>

#include "chess.h"
#include "search.h"

namespace chess
{
    using MoveGenType = BoardState::MoveGenType;

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

    Eval evalFromScore(int score, int searchDepth)
    {
        // In the quiescent search we might find even deeper mates
        constexpr int MAX_QUIESCENT_DEPTH = 100;

        // check if it is not a mate
        if (abs(score) < MATE_EVAL - MAX_QUIESCENT_DEPTH)
            return Eval(Eval::Type::SCORE, score);

        bool whiteMating = score > 0;

        // Search depth that was remaining when the mate was found
        int remainingDepth = abs(score) - MATE_EVAL;

        // remaining depth might be negative if mate is found in quiescent search.
        int mateInPlies = searchDepth - remainingDepth;

        int n = (mateInPlies + 1) / 2;
        return Eval(Eval::Type::MATE, n);
    }

    Search::DepthSettings Search::initialDepths(double thinkSeconds)
    {
        constexpr int MAX_INITIAL_DEPTH = 4;
        float sqrtTime = std::sqrt(thinkSeconds);
        int minDepth = std::min((int)(0.5 * sqrtTime), MAX_INITIAL_DEPTH);

        constexpr int MAX_QUIESCENT_MAX = 12;
        constexpr int MIN_QUIESCENT_MAX = 3;
        int maxQuiescentDepth = sqrtTime / 0.5;
        maxQuiescentDepth = std::min(MAX_QUIESCENT_MAX, std::max(MIN_QUIESCENT_MAX, maxQuiescentDepth));
        minDepth = std::max(1, minDepth);
        return DepthSettings(minDepth, maxQuiescentDepth);
    }

    std::tuple<Move, Eval, Search::SearchStats>
    Search::iterativeDeepening(double thinkSeconds)
    {
        startTimeThread(thinkSeconds);

        Move bestMove = Move::Null();
        int evalScore;
        Eval eval = evalFromScore(0, 0);

        Move currentSearchBest;
        int newScore;

        Search::DepthSettings prevDepths;
        Search::SearchStats prevStats;

        m_depths = initialDepths(thinkSeconds);

        const bool root = true;

        while (eval.type != Eval::Type::MATE)
        {
            // Update the info to the collected info from previous completed search
            prevDepths = m_depths;
            prevStats = m_statistics;

            m_depths.minDepth += 1;
            m_depths.maxQuiescentDepth += 1;

            if (m_rootBoard.whitesMove())
                newScore = minimax<true, root>(m_rootBoard, m_depths.minDepth, currentSearchBest);
            else
                newScore = minimax<false, root>(m_rootBoard, m_depths.minDepth, currentSearchBest);

            // if search is stopped early return using the previous depth results
            if (m_stopped)
            {
                // highest completed depth is one less
                break;
            }

            // only update with each completed search
            evalScore = newScore;
            eval = evalFromScore(evalScore, m_depths.minDepth);
            bestMove = currentSearchBest;
        }

        // The search is done so we stop any still going timer
        stopTimeThread();

        // Set the actually used minDepth
        m_statistics.minDepth = prevDepths.minDepth;

        return {bestMove, eval, m_statistics};
    }

    template <bool Max, bool Root>
    int Search::minimax(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpha, int beta)
    {
        // cancel the search
        if (m_stopped.load(std::memory_order_relaxed))
            return 0;

        // In the root we cannot exit early like this
        // + the curBoard will be the last item in the repTable
        if (!Root && (curBoard.drawBy50MoveRule() || m_repTable->contains(curBoard)))
            return 0; // On repetition we should return draw eval

        // Base case (do a quiescent search)
        if (remainingDepth == 0)
            // return m_evalFunc(curBoard);
            return quiescentSearch<Max>(curBoard, 0, alpha, beta);

        // Start with the worst possible eval
        int bestEval = Max ? INT_MIN : INT_MAX;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Normal>();

        for (const Move &m : pseudoLegalMoves)
        {
            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(Max))
                continue; // skip since move was illegal

            // search with opposite of min/max and not root and 1 less depth
            int moveEval = minimax<!Max, false>(newBoard, remainingDepth - 1, outMove, alpha, beta);

            // Update the bestEval and move only when a strictly better option is found
            // (this prevents using pruned options)
            if (Max ? bestEval < moveEval : bestEval > moveEval)
            {
                bestEval = moveEval;

                if (Root)
                    outMove = m;
            }

            if (Max ? bestEval > beta : bestEval < alpha)
                return bestEval; // The opponent could have chosen a better move in a previous step.

            // max alpha / min beta depending on what player we are
            Max ? alpha = std::max(alpha, bestEval) : beta = std::min(beta, bestEval);
        }

        // Stalemate/mate detection
        bool noLegalMoves = bestEval == INT_MIN || bestEval == INT_MAX;
        if (noLegalMoves)
        {
            // Stalemate is a draw. (if the king is not in check)
            if (!curBoard.kingAttacked(curBoard.whitesMove()))
                return 0;

            // calculate mate evaluation
            // The higher the remaining depth the closer we are to the root (so more negative/positive score).
            bestEval = Max ? -MATE_EVAL - remainingDepth : MATE_EVAL + remainingDepth;
        }

        return bestEval;
    }

    template <bool Max>
    int Search::quiescentSearch(const BoardState &curBoard, int extraDepth, int alpha, int beta)
    {
        // cancel the search
        if (m_stopped.load(std::memory_order_relaxed))
            return 0;

        // Note: no need to check repetition table as each move is a capture (no repetition possible)

        // Update max depth statistic
        m_statistics.reachedDepth = std::max(m_depths.minDepth + extraDepth, m_statistics.reachedDepth);

        // Captures aren't forced so we assume the current positions evaluation as a minimum
        int bestEval = m_evalFunc(curBoard);

        if (m_depths.maxQuiescentDepth <= extraDepth)
            return bestEval;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Quiescent>();

        for (const Move &m : pseudoLegalMoves)
        {
            if (Max ? bestEval > beta : bestEval < alpha)
                return bestEval; // The opponent could have chosen a better move in a previous step.

            // max alpha / min beta depending on what player we are
            Max ? alpha = std::max(alpha, bestEval) : beta = std::min(beta, bestEval);

            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(Max))
                continue; // skip since move was illegal

            // search with opposite of min/max and not root and 1 less depth
            int moveEval = quiescentSearch<!Max>(newBoard, extraDepth + 1, alpha, beta);

            // Update the bestEval and move only when a strictly better option is found
            // (this prevents using pruned options)
            if (Max ? bestEval < moveEval : bestEval > moveEval)
                bestEval = moveEval;
        }

        return bestEval;
    }

    template int Search::minimax<true, true>(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, true>(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpha, int beta);
    template int Search::minimax<true, false>(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpha, int beta);
    template int Search::minimax<false, false>(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpha, int beta);
}