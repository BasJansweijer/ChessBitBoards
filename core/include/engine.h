#pragma once

#include "chess.h"
#include "search.h"
#include "eval.h"
#include <iostream>

namespace chess::engine
{

    Move findBestMove(BoardState &curBoard, int depth)
    {
        Move best;
        int eval;
        if (curBoard.whitesMove())
            eval = minimax<true, true>(curBoard, evaluate, depth, best);
        else
            eval = minimax<false, true>(curBoard, evaluate, depth, best);

        std::cout << "Eval: " << eval << std::endl;
        return best;
    }

}
