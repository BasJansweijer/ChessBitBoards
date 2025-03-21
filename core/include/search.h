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

        // Returns the Move and eval and highest completed depth
        std::tuple<Move, Eval, int> iterativeDeepening(double thinkSeconds);

        // This method is more so used internally, but can also directly be called to search a certain depth.
        template <bool Max, bool Root>
        int minimax(const BoardState &curBoard, int remainingDepth, Move &outMove, int alpa = INT_MIN, int beta = INT_MAX);

    private:
        // Starts a thread which will set m_stopped to true once the specified time has run out
        void startTimeThread(double thinkSeconds);

        // Used to stop the timer early
        void stopTimeThread();

    private:
        const std::function<int(BoardState)> m_evalFunc;
        const BoardState m_rootBoard;
        std::atomic<bool> m_stopped = false;
        std::atomic<bool> m_cancelled = false;
        std::thread m_timerThread;
    };
}