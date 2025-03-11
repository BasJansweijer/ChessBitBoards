#pragma once

#include <functional>
#include "chess.h"
#include "limits.h"

namespace chess::engine
{
    template <bool Max, bool Root>
    int minimax(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpa = INT_MIN, int beta = INT_MAX);

}