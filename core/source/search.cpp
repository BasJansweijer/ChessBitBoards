#include <limits.h>
#include <algorithm>
#include <functional>

#include "chess.h"
#include "search.h"

namespace chess
{
    template <bool Max, bool Root>
    int minimax(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpha, int beta)
    {
        // Base case
        if (depth == 0)
            return eval(curBoard);

        // Start with the worst possible eval
        int bestEval = Max ? INT_MIN : INT_MAX;

        MoveList pseudoLegalMoves = curBoard.pseudoLegalMoves();

        for (const Move &m : pseudoLegalMoves)
        {
            BoardState newBoard = curBoard;
            newBoard.makeMove(m);
            if (newBoard.kingAttacked(Max))
                continue; // skip since move was illegal

            // search with opposite of min/max and not root and 1 less depth
            int moveEval = minimax<!Max, false>(newBoard, eval, depth - 1, outMove, alpha, beta);
            bestEval = Max ? std::max(bestEval, moveEval) : std::min(bestEval, moveEval);

            if (Max ? bestEval > beta : bestEval < alpha)
                break; // The opponent could have chosen a better move in a previous step.

            // max alpha / min beta depending on what player we are
            Max ? alpha = std::max(alpha, bestEval) : beta = std::min(beta, bestEval);

            // Incase this is the root search node then update if we find a better move.
            if (Root && moveEval == bestEval)
                outMove = m;
        }

        return bestEval;
    }

    template int minimax<true, true>(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpha, int beta);
    template int minimax<false, true>(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpha, int beta);
    template int minimax<true, false>(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpha, int beta);
    template int minimax<false, false>(BoardState &curBoard, std::function<int(BoardState)> eval, int depth, Move &outMove, int alpha, int beta);
}