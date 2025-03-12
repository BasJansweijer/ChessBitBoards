#pragma once

#include <functional>
#include "chess.h"
#include "limits.h"
#include <atomic>

namespace chess
{
    class Search
    {
    public:
        Search(BoardState board, std::function<int(BoardState)> eval) : m_rootBoard(board), m_evalFunc(eval) {};

        void stop() { m_stopped = true; }

        // Returns the Move and eval and highest completed depth
        std::tuple<Move, int, int> iterativeDeepening(double thinkSeconds);

        // This method is more so used internally, but can also directly be called to search a certain depth.
        template <bool Max, bool Root>
        int minimax(BoardState &curBoard, int depth, Move &outMove, int alpa = INT_MIN, int beta = INT_MAX);

    private:
        // Starts a thread which will set m_stopped to true once the specified time has run out
        void startTimeThread(double thinkSeconds);

    private:
        std::function<int(BoardState)> m_evalFunc;
        BoardState m_rootBoard;
        std::atomic<bool> m_stopped = false;
    };
}