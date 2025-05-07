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

    void Search::startTimeThread(Time thinkTime)
    {
        m_stopped = false; // Reset before starting
        m_cancelTimer = false;

        // Launch a detached thread to stop search after thinkSeconds
        m_timerThread = std::thread([this, thinkTime]()
                                    {
                                        auto endTime = std::chrono::steady_clock::now() + std::chrono::duration<double, std::milli>(thinkTime);

                                        while (std::chrono::steady_clock::now() < endTime)
                                        {
                                            if (m_cancelTimer)
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
        m_cancelTimer = true;
        m_timerThread.join();
    }

    Search::DepthSettings Search::initialDepths(Time thinkTime)
    {
        constexpr int MAX_INITIAL_DEPTH = 4;
        float sqrtTime = std::sqrt(timeToSeconds(thinkTime));
        int minDepth = std::min((int)(0.5 * sqrtTime), MAX_INITIAL_DEPTH);

        constexpr int MAX_QUIESCENT_MAX = 12;
        constexpr int MIN_QUIESCENT_MAX = 3;
        int maxQuiescentDepth = sqrtTime / 0.5;
        maxQuiescentDepth = std::min(MAX_QUIESCENT_MAX, std::max(MIN_QUIESCENT_MAX, maxQuiescentDepth));
        minDepth = std::max(1, minDepth);
        return DepthSettings(minDepth, maxQuiescentDepth);
    }

    std::tuple<Move, Eval, Search::SearchStats>
    Search::iterativeDeepening(Time thinkTime)
    {
        startTimeThread(thinkTime);

        // Signal to the transposition table that we start a new search (generation)
        m_transTable->startNewSearch();

        // reset bestFoundMove
        m_bestFoundMove = Move::Null();

        int evalScore;
        Eval eval = evalFromScore(0, 0);

        int newScore;

        Search::DepthSettings prevDepths;
        Search::SearchStats prevStats;

        m_depths = initialDepths(thinkTime);

        const bool root = true;

        while (eval.type != Eval::Type::MATE || std::abs(eval.movesTillMate()) >= (m_depths.minDepth + 1) / 2)
        {
            // Update the info to the collected info from previous completed search
            prevDepths = m_depths;
            prevStats = m_statistics;

            m_depths.minDepth += 1;
            m_depths.maxQuiescentDepth += 1;

            int8_t sideToMove = m_rootBoard.whitesMove() ? 1 : -1;
            newScore = minimax<root>(m_rootBoard, m_depths.minDepth) * sideToMove;

            // if search is stopped early return using the previous depth results
            // If we are stopped and the minDepth is greater than MAX_SEARCH_DEPTH we are probably
            // dealing with some weird position (searching this deep doesn't help so we stop)
            if (m_stopped || m_depths.maxQuiescentDepth >= MAX_SEARCH_DEPTH)
            {
                // highest completed depth is one less
                m_depths.minDepth -= 1;
                m_depths.maxQuiescentDepth -= 1;
                break;
            }

            // only update with each completed search
            evalScore = newScore;
            eval = evalFromScore(evalScore, m_depths.minDepth);
        }

        // The search is done so we stop any still going timer
        stopTimeThread();

        // Set the actually used minDepth
        m_statistics.minDepth = prevDepths.minDepth;

        return {m_bestFoundMove, eval, m_statistics};
    }

    // Uses RAII to pop and add a board to the repetition table
    class RepetitionScope
    {
    public:
        RepetitionScope(RepetitionTable *repTable, BoardState b)
            : m_repTable(repTable)
        {
            m_repTable->addState(b);
        }

        ~RepetitionScope()
        {
            m_repTable->pop();
        }

    private:
        RepetitionTable *m_repTable;
    };

    template <bool Root>
    score Search::minimax(const BoardState &curBoard, int remainingDepth, score alpha, score beta)
    {
        // cancel the search
        if (stopSearch())
            return 0;

        // update searched node count
        m_statistics.searchedNodes++;

        // In the root we cannot exit early like this
        if (!Root && (m_repTable->drawBy50MoveRule() || m_repTable->contains(curBoard)))
            return 0; // On repetition we should return draw eval

        // Base case (do a quiescent search)
        if (remainingDepth == 0)
            return quiescentSearch(curBoard, 0, alpha, beta);

        uint8_t curDepth = m_depths.minDepth - remainingDepth;

        // Look in the transposition table for a usable entry for this board
        key boardHash = curBoard.getHash();
        TTEntry *transEntry = m_transTable->get(boardHash);
        bool containsCurBoard = transEntry->containsHash(boardHash);
        if (containsCurBoard)
        {
            // In the root we need to return a move so we can't return like this
            // TODO: return move if root
            if (transEntry->evalUsable(curDepth, remainingDepth, alpha, beta))
            {
                score rootEval = scoreForRootNode(transEntry->eval, curDepth);
                if constexpr (!Root)
                    return rootEval; // use evaluation emediately

                // if this is the root we need to first set the found move
                m_bestFoundMove = transEntry->move;
                // and then return the score
                return rootEval;
            }
        }

        // get the move from the transposition table if available
        Move TTMove = containsCurBoard ? transEntry->move : Move::Null();
        // Either we should reference the search result or a local move (not at root)
        Move &bestMove = Root ? m_bestFoundMove : TTMove;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Normal>();
        // order the moves to improve pruning
        m_moveScorer.orderMoves(pseudoLegalMoves, curBoard, TTMove);

        // add the current board to the repetition table
        // we use RAII to automatically pop it again when we exit this depth
        RepetitionScope repStateRAII = RepetitionScope(m_repTable, curBoard);

        // Start with the worst possible eval
        score bestEval = SCORE_MIN;
        score originalAlpha = alpha;
        bool firstMove = true;
        bool evalFromFullSearch = false;
        for (const Move &m : pseudoLegalMoves)
        {
            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(curBoard.whitesMove()))
                continue; // skip since move was illegal

            score moveEval;
            if (!firstMove)
            {
                // PVS null/zero window search
                score nextBeta = -alpha;
                moveEval = -minimax<false>(newBoard, remainingDepth - 1, nextBeta - 1, nextBeta);
                // check if we need a full search
                evalFromFullSearch = moveEval > alpha && beta - alpha > 1;
                if (evalFromFullSearch)
                    // full search
                    moveEval = moveEval = -minimax<false>(newBoard, remainingDepth - 1, -beta, -alpha);
            }
            else // is firstMove
            {
                // full search
                moveEval = moveEval = -minimax<false>(newBoard, remainingDepth - 1, -beta, -alpha);
                evalFromFullSearch = true;
                firstMove = false;
            }

            if (stopSearch())
                // if the search is stopped we need to return to prevent using this moveEval result
                return 0;

            // Update the bestEval and move only when a strictly better option is found
            // (this prevents using pruned options)
            if (bestEval < moveEval)
            {
                bestEval = moveEval;
                bestMove = m;
            }

            if (bestEval > beta)
            { // cut-off (The opponent could have chosen a better move in a previous step.)
                bestMove = m;

                // we register the move producing the cut off to improve future move ordering
                m_moveScorer.registerBetaCutOff(m, remainingDepth);
                break;
            }

            // max alpha (we use negamax so alpha=-beta on next depth)
            alpha = std::max(alpha, bestEval);
        }

        // Stalemate/mate detection
        bool noLegalMoves = bestEval == SCORE_MIN;
        if (noLegalMoves)
        {
            // Stalemate if the king is not in check)
            bool isStalemate = !curBoard.kingAttacked(curBoard.whitesMove());
            return isStalemate
                       ? 0                           // stalemate
                       : -MAX_MATE_SCORE + curDepth; // calculate mate evaluation
        }

        /*
         * Store the evaluation in the transposition table
         */
        EvalBound bound;

        if (bestEval > beta)
            bound = EvalBound::Lower; // Beta cutoff (fail-high, could be higher)
        else if (bestEval < originalAlpha)
            bound = EvalBound::Upper; // Alpha cutoff (fail-low, could be lower)
        else if (!evalFromFullSearch)
            // even though we are between alpha and beta the null window still caused a alpha cutoff (likely)
            bound = EvalBound::Upper;
        else
            bound = EvalBound::Exact; // Full search was done

        score eval = scoreForCurrentNode(bestEval, curDepth);

        m_transTable->set(boardHash, TTEntry(eval, remainingDepth, bound, bestMove));

        return bestEval;
    }

    score Search::quiescentSearch(const BoardState &curBoard, int extraDepth, score alpha, score beta)
    {
        // cancel the search
        if (stopSearch())
            return 0;

        // update searched node count
        m_statistics.searchedNodes++;

        // Note: no need to check repetition table as each move is a capture (no repetition possible)

        uint8_t curDepth = m_depths.minDepth + extraDepth;

        // Look in the transposition table for a usable entry for this board
        key boardHash = curBoard.getHash();
        TTEntry *transEntry = m_transTable->get(boardHash);
        bool containsCurBoard = transEntry->containsHash(boardHash);
        if (containsCurBoard)
        {
            // remaining depth is zero
            if (transEntry->evalUsable(curDepth, 0, alpha, beta))
                return scoreForRootNode(transEntry->eval, curDepth);
        }

        // get the move from the transposition table if available
        Move TTMove = containsCurBoard ? transEntry->move : Move::Null();

        // Update max depth statistic
        m_statistics.reachedDepth = std::max(curDepth, m_statistics.reachedDepth);

        // Captures aren't forced so we assume the current positions evaluation as a minimum
        // scale to current players perspective (negamax)
        int8_t sideToMove = (curBoard.whitesMove() ? 1 : -1);
        score bestEval = m_evalFunc(curBoard) * sideToMove;

        if (m_depths.maxQuiescentDepth <= extraDepth)
            return bestEval;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Quiescent>();
        // order the moves to improve pruning
        m_moveScorer.orderMoves(pseudoLegalMoves, curBoard, TTMove);

        for (const Move &m : pseudoLegalMoves)
        {
            if (bestEval > beta)
                return bestEval; // The opponent could have chosen a better move in a previous step.

            // max alpha (alpha == -beta on next recursion)
            alpha = std::max(alpha, bestEval);

            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(curBoard.whitesMove()))
                continue; // skip since move was illegal

            // negamax recursion (next depth)
            int moveEval = -quiescentSearch(newBoard, extraDepth + 1, -beta, -alpha);

            // Update the bestEval and move only when a strictly better option is found
            // (this prevents using pruned options)
            if (bestEval < moveEval)
                bestEval = moveEval;
        }

        return bestEval;
    }

    template score Search::minimax<true>(const BoardState &curBoard, int remainingDepth, score alpha, score beta);
    template score Search::minimax<false>(const BoardState &curBoard, int remainingDepth, score alpha, score beta);
}