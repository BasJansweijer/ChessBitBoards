/*
This file implements all Evaluator functions that are called to initialize the internal variables
*/

#include "eval.h"
#include "evalTables.h"
#include "bitBoard.h"
#include "chess.h"
#include "types.h"

namespace chess
{

    /*
     * Initialization Helper functions
     */
    void Evaluator::calculateMaterial()
    {
        whiteMaterial = 0;
        blackMaterial = 0;

        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            whitePieceCounts[pieceType] = bitBoards::bitCount(whiteBitBoards[pieceType]);
            whiteMaterial += whitePieceCounts[pieceType] * pieceVals[pieceType];
            blackPieceCounts[pieceType] = bitBoards::bitCount(blackBitBoards[pieceType]);
            blackMaterial += blackPieceCounts[pieceType] * pieceVals[pieceType];
        }

        score whiteNonPawnMaterial = whiteMaterial - whitePieceCounts[PieceType::Pawn] * pieceVals[PieceType::Pawn];
        score blackNonPawnMaterial = blackMaterial - blackPieceCounts[PieceType::Pawn] * pieceVals[PieceType::Pawn];

        // Calculate the percentage of (non pawn) pieces that is remaining
        piecesMaterialLeft = (whiteNonPawnMaterial + blackNonPawnMaterial) / (float)(startingPieceMaterial * 2);
    }

    // Arguably not initialization, but sets the middleGame and endGame scores
    void Evaluator::calculatePieceSquareTableScores()
    {
        middleGameScore = 0;
        endGameScore = 0;

        // Go through every piece (except king)
        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            bitBoards::forEachBit(whiteBitBoards[pieceType], [&](square s)
                                  { 
                middleGameScore += evalTables::middleGameWhite[pieceType][s];
                endGameScore += evalTables::endGameBlack[pieceType][s]; });

            bitBoards::forEachBit(blackBitBoards[pieceType], [&](square s)
                                  { 
                middleGameScore -= evalTables::middleGameBlack[pieceType][s];
                endGameScore -= evalTables::endGameBlack[pieceType][s]; });
        }

        // Add the king position score
        square whiteKing = board.getWhiteKingSquare();
        square blackKing = board.getBlackKingSquare();
        middleGameScore += evalTables::middleGameWhite[PieceType::King][whiteKing];
        middleGameScore -= evalTables::middleGameBlack[PieceType::King][blackKing];
        endGameScore += evalTables::endGameWhite[PieceType::King][whiteKing];
        endGameScore -= evalTables::endGameBlack[PieceType::King][blackKing];
    }

    /*
    Using the material of both players we determine a number in the range [0, 1]
    Where 0 means, this is not at all an endgame position and 1 means this is an endgame position.
    NOTE: we use "static" global pieceCount arrays for this
    */
    void Evaluator::calculateEndGameNess()
    {
        // Remove pawns from the equation
        int whitePieceMaterial = whiteMaterial - pieceVals[PieceType::Pawn] * whitePieceCounts[PieceType::Pawn];
        int blackPieceMaterial = blackMaterial - pieceVals[PieceType::Pawn] * blackPieceCounts[PieceType::Pawn];

        // Value the queen as more than usual
        whitePieceMaterial += whitePieceCounts[PieceType::Queen] * 300;
        blackPieceMaterial += whitePieceCounts[PieceType::Queen] * 300;

        // an offset of the highest material a side may have in an endgame
        constexpr int maxEndGameMaterial = pieceVals[PieceType::Rook] * 2;

        // A divisor such that when no pieces are taken we return 1
        constexpr float divisor = startingPieceMaterial * 2 - maxEndGameMaterial * 2;

        float score = (whitePieceMaterial + blackPieceMaterial - 2 * maxEndGameMaterial) / divisor;
        // normalize score to let 1 be completely endgame and 0 be completely not endgame
        score = score < 0 ? 1 : 1 - score;

        // We run the score through a function which ensures that we "skew" more towards 1.
        // Prevents us from using endgame tables too early whilst still using them almost entirely when
        // the score gets lower
        float scoreSquare = score * score;
        endGameNessScore = scoreSquare * scoreSquare;
    }

    // Initializes the fileTypes array
    void Evaluator::determineOpenFiles()
    {
        // Set file types
        for (int i = 0; i < 8; i++)
        {
            fileTypes[i] = FileType::CLOSED;
            if (!(board.getWhitePawns() & bitBoards::fileMask(i)))
                fileTypes[i] |= FileType::HALF_OPEN_WHITE;
            if (!(board.getBlackPawns() & bitBoards::fileMask(i)))
                fileTypes[i] |= FileType::HALF_OPEN_BLACK;
        }
    }
}