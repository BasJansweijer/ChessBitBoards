#include <limits.h>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <cmath>

#include "chess.h"
#include "search.h"

#include "boardVisualizer.h"

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

    // Is used to order the moves in the move list
    // this increases the performance of the search as we can prune more
    score Search::moveScore(const Move &move, const BoardState &board) const
    {
        // We want to always try promotion to queen early on in the search
        constexpr score queenPromotionValue = pieceVals[Queen];
        if (move.isPromotion() && move.piece == Queen)
            return queenPromotionValue;

        // not a promotion to a queen or a capture
        if (!move.isCapture())
            return 0;

        score capturingPieceValue = pieceVals[move.piece];
        PieceType capturedPiece = board.whitesMove() ? board.pieceOnSquare<false>(move.to) : board.pieceOnSquare<true>(move.to);
        score differenceInValue = pieceVals[capturedPiece] - capturingPieceValue;
        // we assume the capture is save (but slightly prefer taking with a lower value piece)
        score moveScore = pieceVals[capturedPiece] + (differenceInValue / 50);
        return moveScore;
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

        // Signal to the transposition table that we start a new search (generation)
        m_transTable->startNewSearch();

        Move bestMove = Move::Null();
        int evalScore;
        Eval eval = evalFromScore(0, 0);

        Move currentSearchBest;
        int newScore;

        Search::DepthSettings prevDepths;
        Search::SearchStats prevStats;

        m_depths = initialDepths(thinkSeconds);

        const bool root = true;

        while (eval.type != Eval::Type::MATE || std::abs(eval.movesTillMate()) >= (m_depths.minDepth + 1) / 2)
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
            // If we are stopped and the minDepth is greater than 1000 we are probably dealing with
            // a repetition in the search tree.
            if (m_stopped || m_depths.minDepth > 1000)
            {
                // highest completed depth is one less
                m_depths.minDepth -= 1;
                m_depths.maxQuiescentDepth -= 1;
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
    score Search::minimax(const BoardState &curBoard, int remainingDepth, Move &outMove, score alpha, score beta)
    {
        // cancel the search
        if (m_stopped.load(std::memory_order_relaxed))
            return 0;

        // update searched node count
        m_statistics.searchedNodes++;

        // In the root we cannot exit early like this
        // + the curBoard will be the last item in the repTable
        if (!Root && (curBoard.drawBy50MoveRule() || m_repTable->contains(curBoard)))
            return 0; // On repetition we should return draw eval

        // Base case (do a quiescent search)
        if (remainingDepth == 0)
            return quiescentSearch<Max>(curBoard, 0, alpha, beta);

        uint8_t curDepth = m_depths.minDepth - remainingDepth;

        // Look in the transposition table for a usable entry for this board
        key boardHash = curBoard.getHash();
        TTEntry *transEntry = m_transTable->get(boardHash);

        bool containsCurBoard = transEntry->containsHash(boardHash);
        if (containsCurBoard)
        {
            // In the root we need to return a move so we can't return like this
            // TODO: return move if root
            if (!Root && transEntry->evalUsable(curDepth, remainingDepth, alpha, beta))
                return scoreForRootNode(transEntry->eval, curDepth); // use evaluation emediately
        }

        // get the move from the transposition table if available
        Move TTMove = containsCurBoard ? transEntry->move : Move::Null();

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Normal>();
        // order the moves to improve pruning
        orderMoves(pseudoLegalMoves, curBoard, TTMove);

        // Start with the worst possible eval
        score bestEval = Max ? SCORE_MIN : SCORE_MAX;
        Move bestMove = Move::Null();

        for (const Move &m : pseudoLegalMoves)
        {
            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(Max))
                continue; // skip since move was illegal

            // search with opposite of min/max and not root and 1 less depth
            score moveEval = minimax<!Max, false>(newBoard, remainingDepth - 1, outMove, alpha, beta);

            // Update the bestEval and move only when a strictly better option is found
            // (this prevents using pruned options)
            if (Max ? bestEval < moveEval : bestEval > moveEval)
            {
                bestEval = moveEval;
                bestMove = m;

                if (Root)
                    // TODO: remove "outMove" parameter
                    outMove = m;
            }

            if (Max ? bestEval > beta : bestEval < alpha)
            {
                bestMove = m;
                break; // The opponent could have chosen a better move in a previous step.
            }

            // max alpha / min beta depending on what player we are
            Max ? alpha = std::max(alpha, bestEval) : beta = std::min(beta, bestEval);
        }

        // Stalemate/mate detection
        bool noLegalMoves = bestEval == SCORE_MIN || bestEval == SCORE_MAX;
        if (noLegalMoves)
        {
            // Stalemate is a draw. (if the king is not in check)
            if (!curBoard.kingAttacked(curBoard.whitesMove()))
                return 0;

            // calculate mate evaluation
            // The higher the depth the closer the score it to zero
            bestEval = Max ? -MAX_MATE_SCORE + curDepth : MAX_MATE_SCORE - curDepth;
        }

        if (m_stopped.load(std::memory_order_relaxed))
            // if the search was cancelled don't store the results as they are 'faulty'.
            return bestEval;

        /*
         * Store the evaluation in the transposition table
         */
        EvalBound bound;
        if (bestEval > beta)
            bound = EvalBound::Lower; // Beta cutoff (fail-high, could be higher)
        else if (bestEval < alpha)
            bound = EvalBound::Upper; // Alpha cutoff (fail-low, could be lower)
        else
            bound = EvalBound::Exact; // Full search was done

        score eval = scoreForCurrentNode(bestEval, curDepth);

        m_transTable->set(boardHash, TTEntry(eval, remainingDepth, bound, bestMove));

        return bestEval;
    }

    template <bool Max>
    score Search::quiescentSearch(const BoardState &curBoard, int extraDepth, score alpha, score beta)
    {
        // cancel the search
        if (m_stopped.load(std::memory_order_relaxed))
            return 0;

        // update searched node count
        m_statistics.searchedNodes++;

        // Note: no need to check repetition table as each move is a capture (no repetition possible)

        int curDepth = m_depths.minDepth + extraDepth;
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
        m_statistics.reachedDepth = std::max(m_depths.minDepth + extraDepth, m_statistics.reachedDepth);

        // Captures aren't forced so we assume the current positions evaluation as a minimum
        score bestEval = m_evalFunc(curBoard);

        if (m_depths.maxQuiescentDepth <= extraDepth)
            return bestEval;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves<MoveGenType::Quiescent>();
        // order the moves to improve pruning
        orderMoves(pseudoLegalMoves, curBoard, TTMove);

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

    template score Search::minimax<true, true>(const BoardState &curBoard, int remainingDepth, Move &outMove, score alpha, score beta);
    template score Search::minimax<false, true>(const BoardState &curBoard, int remainingDepth, Move &outMove, score alpha, score beta);
    template score Search::minimax<true, false>(const BoardState &curBoard, int remainingDepth, Move &outMove, score alpha, score beta);
    template score Search::minimax<false, false>(const BoardState &curBoard, int remainingDepth, Move &outMove, score alpha, score beta);
}