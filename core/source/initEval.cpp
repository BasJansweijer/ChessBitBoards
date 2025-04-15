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
        m_whitePieceMaterial = 0;
        m_blackPieceMaterial = 0;

        for (int pieceType = 1; pieceType < 5; pieceType++)
        {
            m_whitePieceCounts[pieceType] = bitBoards::bitCount(m_whiteBitBoards[pieceType]);
            m_whitePieceMaterial += m_whitePieceCounts[pieceType] * pieceVals[pieceType];
            m_blackPieceCounts[pieceType] = bitBoards::bitCount(m_blackBitBoards[pieceType]);
            m_blackPieceMaterial += m_blackPieceCounts[pieceType] * pieceVals[pieceType];
        }

        // Set the material of the pawns
        m_whitePieceCounts[PieceType::Pawn] = bitBoards::bitCount(m_whiteBitBoards[PieceType::Pawn]);
        m_blackPieceCounts[PieceType::Pawn] = bitBoards::bitCount(m_blackBitBoards[PieceType::Pawn]);

        // Calculate the percentage of (non pawn) pieces that is remaining
        m_piecesMaterialLeft = (m_whitePieceMaterial + m_blackPieceMaterial) / (float)(startingPieceMaterial * 2);
    }

    // Arguably not initialization, but sets the middleGame and endGame scores
    void Evaluator::calculatePieceSquareTableScores()
    {
        m_middleGameScore = 0;
        m_endGameScore = 0;

        // Go through every piece (except king)
        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            bitBoards::forEachBit(m_whiteBitBoards[pieceType], [&](square s)
                                  { 
                m_middleGameScore += evalTables::middleGameWhite[pieceType][s];
                m_endGameScore += evalTables::endGameBlack[pieceType][s]; });

            bitBoards::forEachBit(m_blackBitBoards[pieceType], [&](square s)
                                  { 
                m_middleGameScore -= evalTables::middleGameBlack[pieceType][s];
                m_endGameScore -= evalTables::endGameBlack[pieceType][s]; });
        }

        // Add the king position score
        square whiteKing = m_board.getWhiteKingSquare();
        square blackKing = m_board.getBlackKingSquare();
        m_middleGameScore += evalTables::middleGameWhite[PieceType::King][whiteKing];
        m_middleGameScore -= evalTables::middleGameBlack[PieceType::King][blackKing];
        m_endGameScore += evalTables::endGameWhite[PieceType::King][whiteKing];
        m_endGameScore -= evalTables::endGameBlack[PieceType::King][blackKing];
    }

    /*
    Using the material of both players we determine a number in the range [0, 1]
    Where 0 means, this is not at all an endgame position and 1 means this is an endgame position.
    NOTE: we use "static" global pieceCount arrays for this
    */
    void Evaluator::calculateEndGameNess()
    {
        // Value the queen as more than usual
        m_whitePieceMaterial += m_whitePieceCounts[PieceType::Queen] * 300;
        m_blackPieceMaterial += m_whitePieceCounts[PieceType::Queen] * 300;

        // an offset of the highest material a side may have in an endgame
        constexpr int maxEndGameMaterial = pieceVals[PieceType::Rook] * 2;

        // A divisor such that when no pieces are taken we return 1
        constexpr float divisor = startingPieceMaterial * 2 - maxEndGameMaterial * 2;

        float score = (m_whitePieceMaterial + m_blackPieceMaterial - 2 * maxEndGameMaterial) / divisor;
        // normalize score to let 1 be completely endgame and 0 be completely not endgame
        score = score < 0 ? 1 : 1 - score;

        // We run the score through a function which ensures that we "skew" more towards 1.
        // Prevents us from using endgame tables too early whilst still using them almost entirely when
        // the score gets lower
        float scoreSquare = score * score;
        m_endGameNessScore = scoreSquare * scoreSquare;
    }

    // Initializes the fileTypes array
    void Evaluator::determineOpenFiles()
    {
        // Set file types
        for (int i = 0; i < 8; i++)
        {
            m_fileTypes[i] = FileType::CLOSED;
            if (!(m_board.getWhitePawns() & bitBoards::fileMask(i)))
                m_fileTypes[i] |= FileType::HALF_OPEN_WHITE;
            if (!(m_board.getBlackPawns() & bitBoards::fileMask(i)))
                m_fileTypes[i] |= FileType::HALF_OPEN_BLACK;
        }
    }
}