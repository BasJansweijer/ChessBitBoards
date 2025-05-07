#include "moveOrdering.h"
#include "eval.h"

namespace chess
{
    score captureScore(const Move &move, const BoardState &board)
    {
        score capturingPieceValue = pieceVals[move.piece];
        PieceType capturedPiece = board.whitesMove() ? board.pieceOnSquare<false>(move.to) : board.pieceOnSquare<true>(move.to);
        score differenceInValue = pieceVals[capturedPiece] - capturingPieceValue;
        // we assume the capture is save (but slightly prefer taking with a lower value piece)
        score moveScore = pieceVals[capturedPiece] + (differenceInValue / 50);
        return moveScore;
    }
    // Is used to order the moves in the move list
    // this increases the performance of the search as we can prune more
    score MoveScorer::moveScore(Move move, const BoardState &board, Move prevMove) const
    {
        // if a quiet move has the maximum value in the history table we still order it before our bad captures
        constexpr score NON_QUIET_OFFSET = TABLE_MAX - 800;
        // We want to always try promotion to queen early on in the search
        constexpr score queenPromotionValue = pieceVals[Queen];
        if (move.isPromotion() && move.piece == Queen)
            return queenPromotionValue + NON_QUIET_OFFSET;

        // not a promotion to a queen or a capture
        if (move.isCapture())
            return captureScore(move, board) + NON_QUIET_OFFSET;

        uint16_t idx = moveIdx(move);
        uint16_t prevMoveIdx = moveIdx(prevMove);

        // This has been tuned a bit by looking at the nodes searched at depth 8 on the starting position
        constexpr float HISTORY_WEIGHT = 0.85;
        constexpr float COUNTER_WEIGHT = 0.15;
        score moveScore = m_historyTable[idx] * HISTORY_WEIGHT +
                          m_counterMoveTable[prevMoveIdx][idx] * COUNTER_WEIGHT;

        return moveScore;
    }
}