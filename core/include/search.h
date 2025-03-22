#pragma once

#include <functional>
#include "chess.h"
#include "limits.h"
#include <thread>
#include <atomic>
#include "eval.h"

namespace chess
{
    class Search
    {
    public:
        /*
        Initializes the search on a certain board state.

        IMPORTANT: the eval function should return ints between [-MATE_EVAL, MATE_EVAL].
        The evaluations that are bellow and above this are used for mate in 0 through mate in MAX_DEPTH.
        */
        Search(BoardState board, std::function<int(BoardState)> eval) : m_rootBoard(board), m_evalFunc(eval) {};

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

        // Used for hard limits on search depth etc.
        struct SearchSettings
        {
            int minDepth = 1;          // The depth the full minimax search should go to.
            int maxQuiescentDepth = 0; // The maximum extra steps for the quiescent search

            SearchSettings() = default;
            SearchSettings(int minDepth, int maxQuiescentDepth)
                : minDepth(minDepth), maxQuiescentDepth(maxQuiescentDepth) {}
        };

        // Returns the Move and eval and highest completed depth
        std::tuple<Move, Eval, SearchStats> iterativeDeepening(double thinkSeconds);

        // This method is more so used internally, but can also directly be called to search a certain depth.
        template <bool Max, bool Root>
        int minimax(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpa = INT_MIN, int beta = INT_MAX);

    private:
        // does a search only using captures (MoveGenType::Quiescent)
        template <bool Max>
        int quiescentSearch(const BoardState &curBoard, int extraDepth, int alpha = INT_MIN, int beta = INT_MAX);

        // Starts a thread which will set m_stopped to true once the specified time has run out
        void startTimeThread(double thinkSeconds);

        // Used to stop the timer early
        void stopTimeThread();

    private:
        const std::function<int(BoardState)> m_evalFunc;
        const BoardState m_rootBoard;

        // Handles the limits of the search
        SearchSettings m_settings;
        // tracks the actual search depth etc
        SearchStats m_statistics;

        std::atomic<bool> m_stopped = false;
        std::atomic<bool> m_cancelled = false;
        std::thread m_timerThread;
    };
}