#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <stdexcept>

#include "types.h"
#include "chess.h"
#include "limits.h"
#include "eval.h"
#include "repetitionTable.h"
#include "transposition.h"
#include "timeman.h"

namespace chess
{
    class Search
    {
    public:
        struct SearchConfig
        {
            std::function<score(const BoardState &)> evalFunction;
            RepetitionTable *repTable = nullptr;
            TranspositionTable *transTable = nullptr;

            SearchConfig() = default;

            SearchConfig(std::function<int(const BoardState &)> evalFunc, RepetitionTable *const rt = nullptr)
                : evalFunction(evalFunc), repTable(rt)
            {
            }
        };

        /*
        Initializes the search on a certain board state.

        IMPORTANT: the eval function should return ints between [-MATE_EVAL, MATE_EVAL].
        The evaluations that are bellow and above this are used for mate in 0 through mate in MAX_DEPTH.
        */
        Search(BoardState board, SearchConfig config)
            : m_rootBoard(board), m_evalFunc(config.evalFunction),
              m_repTable(config.repTable), m_transTable(config.transTable)
        {
            // If no repetition table is given we use an empty "dummy" table as a placeholder
            if (m_repTable == nullptr)
                throw std::runtime_error("Missing repetition table in search config");

            if (m_transTable == nullptr)
                throw std::runtime_error("Missing transposition table in search config");

            // initialize the history table to all zeros
            std::fill(m_moveHistoryTable, &m_moveHistoryTable[Move::MAX_MOVE_IDX + 1], 0);
        };

        void stop() { m_stopped = true; }

        // Used for tracking of actual search
        struct SearchStats
        {
            // Depth untill which everything is explored
            int minDepth = 0;
            // Maximum depth including quiescent search
            int reachedDepth = 0;
            int searchedNodes = 0;

            // Overload operator<< for printing
            friend std::ostream &operator<<(std::ostream &os, const SearchStats &info)
            {
                os << "{ minDepth=" << info.minDepth
                   << ", maxDepth=" << info.reachedDepth
                   << ", nodesSearched=" << info.searchedNodes << "}";
                return os;
            }
        };

        SearchStats getStats() const
        {
            return m_statistics;
        }

        // Returns the Move and eval and highest completed depth
        std::tuple<Move, Eval, SearchStats> iterativeDeepening(Time thinkTime);

        // This method is more so used internally, but can also directly be called to search a certain depth.
        template <bool Root>
        score minimax(const BoardState &curBoard, int remainingDepth, score alpha = SCORE_MIN, score beta = SCORE_MAX);

    private:
        // Used for hard limits on search depth etc.
        struct DepthSettings
        {
            int minDepth = 1;          // The depth the full minimax search should go to.
            int maxQuiescentDepth = 0; // The maximum extra steps for the quiescent search

            DepthSettings() = default;
            DepthSettings(int minDepth, int maxQuiescentDepth)
                : minDepth(minDepth), maxQuiescentDepth(maxQuiescentDepth) {}
        };

        DepthSettings initialDepths(Time thinkTime);

        // does a search only using captures (MoveGenType::Quiescent)
        score quiescentSearch(const BoardState &curBoard, int extraDepth, score alpha = SCORE_MIN, score beta = SCORE_MAX);

        // Starts a thread which will set m_stopped to true once the specified time has run out
        void startTimeThread(Time thinkTime);

        // Used to stop the timer early
        void stopTimeThread();

        // Orders the moves in the move list
        // We also use the transposition table to get the best move
        // from the previous search and put it at the front of the list
        inline void orderMoves(MoveList &moves, const BoardState &board, const Move &TTMove, int curDepth)
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

        // Gives a rough heuristic based on which we can order the moves in our search
        // Put in this class because we can use info from the search to help the ordering
        score moveScore(const Move &move, const BoardState &board) const;

        inline bool stopSearch() const
        {
            return m_stopped.load(std::memory_order_relaxed);
        }

    private:
        const std::function<score(const BoardState &)> m_evalFunc;
        // Repetition table passed down by the engine class
        RepetitionTable *m_repTable;
        TranspositionTable *m_transTable;
        uint16_t m_moveHistoryTable[Move::MAX_MOVE_IDX + 1];

        const BoardState m_rootBoard;
        // current best found move:
        Move m_bestFoundMove;

        // Handles the limits of the search
        DepthSettings m_depths;
        // tracks the actual search depth etc
        SearchStats m_statistics;

        std::atomic<bool> m_stopped = false;
        std::atomic<bool> m_cancelTimer = false;
        std::thread m_timerThread;
    };

}