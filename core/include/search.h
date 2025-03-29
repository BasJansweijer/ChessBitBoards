#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include "chess.h"
#include "limits.h"
#include "eval.h"
#include "repetitionTable.h"

namespace chess
{

    class Search
    {
    public:
        struct SearchConfig
        {
            std::function<int(BoardState)> evalFunction;
            const RepetitionTable *repTable = nullptr;

            SearchConfig() = default;

            SearchConfig(std::function<int(BoardState)> evalFunc, const RepetitionTable *const rt = nullptr)
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
            : m_rootBoard(board), m_evalFunc(config.evalFunction), m_repTable(config.repTable)
        {
            // If no repetition table is given we use an empty "dummy" table as a placeholder
            if (m_repTable == nullptr)
                m_repTable = new RepetitionTable();
        };

        void stop() { m_stopped = true; }

        // Used for tracking of actual search
        struct SearchStats
        {
            // Depth untill which everything is explored
            int minDepth = 0;
            // Maximum depth including quiescent search
            int reachedDepth = 0;

            // Overload operator<< for printing
            friend std::ostream &operator<<(std::ostream &os, const SearchStats &info)
            {
                os << "{ minDepth=" << info.minDepth << ", maxDepth=" << info.reachedDepth << "}";
                return os;
            }
        };

        // Returns the Move and eval and highest completed depth
        std::tuple<Move, Eval, SearchStats> iterativeDeepening(double thinkSeconds);

        // This method is more so used internally, but can also directly be called to search a certain depth.
        template <bool Max, bool Root>
        int minimax(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpa = INT_MIN, int beta = INT_MAX);

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

        DepthSettings initialDepths(double thinkSeconds);

        // does a search only using captures (MoveGenType::Quiescent)
        template <bool Max>
        int quiescentSearch(const BoardState &curBoard, int extraDepth, int alpha = INT_MIN, int beta = INT_MAX);

        // Starts a thread which will set m_stopped to true once the specified time has run out
        void startTimeThread(double thinkSeconds);

        // Used to stop the timer early
        void stopTimeThread();

    private:
        const std::function<int(BoardState)> m_evalFunc;
        // Repetition table passed down by the engine class
        const RepetitionTable *m_repTable;
        const BoardState m_rootBoard;

        // Handles the limits of the search
        DepthSettings m_depths;
        // tracks the actual search depth etc
        SearchStats m_statistics;

        std::atomic<bool> m_stopped = false;
        std::atomic<bool> m_cancelled = false;
        std::thread m_timerThread;
    };
}