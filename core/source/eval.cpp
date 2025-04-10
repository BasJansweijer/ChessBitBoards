#include "eval.h"
#include "bitBoard.h"
#include "moveConstants.h"
#include <cinttypes>
#include <algorithm>
#include "evalTables.h"

namespace tables = chess::evalTables;

namespace chess
{
    score Evaluator::kingSafety()
    {
        constexpr int KING_SAFETY_MULT = 10;
        square blackKing = board.getBlackKingSquare();
        square whiteKing = board.getWhiteKingSquare();

        bitboard whitePawns = board.getWhitePawns();
        bitboard blackPawns = board.getBlackPawns();

        bitboard whiteMoves = constants::getBishopMoves(whiteKing, whitePawns) | constants::getRookMoves(whiteKing, whitePawns);
        whiteMoves &= ~(whitePawns && bitBoards::rankMask(0));
        int whiteSafetyScore = bitBoards::bitCount(whiteMoves) * -KING_SAFETY_MULT;

        bitboard blackMoves = constants::getBishopMoves(blackKing, blackPawns) | constants::getRookMoves(blackKing, blackPawns);
        blackMoves &= ~(whitePawns && bitBoards::rankMask(7));
        int blackSafetyScore = bitBoards::bitCount(blackMoves) * KING_SAFETY_MULT;

        return blackSafetyScore + whiteSafetyScore;
    }

    // mopup score strongly influences the normal evaluation at the very end of the game to promote cornering the losing king
    float Evaluator::mopUpFactor()
    {
        // If a player is up this much we might have to mop up.
        constexpr int minMaterialDiff = pieceVals[PieceType::Rook] - 2 * pieceVals[PieceType::Pawn];

        const int materialAdvantage = std::abs(whiteMaterial - blackMaterial);

        // We factor is lower when opponent has more material still on the board
        const int opponentMaterial = whiteMaterial > blackMaterial ? blackMaterial : whiteMaterial;

        return materialAdvantage < minMaterialDiff || endGameNessScore < 0.5
                   ? 0 // don't use mopup score
                   : materialAdvantage * endGameNessScore / (float)(pieceVals[PieceType::Rook] + opponentMaterial);
    }

    score Evaluator::mopUpScore()
    {
        square whiteKing = board.getWhiteKingSquare();
        square blackKing = board.getBlackKingSquare();
        int8_t wKingRank = whiteKing / 8;
        int8_t wKingFile = whiteKing % 8;
        int8_t bKingRank = blackKing / 8;
        int8_t bKingFile = blackKing % 8;

        bool whiteToWin = getMaterialBalance() > 0;
        uint8_t *matingPieceCounts = whiteToWin ? whitePieceCounts : blackPieceCounts;

        constexpr int centerDistMult = 20;
        constexpr int kingDistMult = 5;

        square losingKing = whiteToWin ? blackKing : whiteKing;
        int centerDistTerm = tables::centerManhattanDistance[losingKing] * centerDistMult;
        uint8_t kingDist = abs(wKingRank - bKingRank) + abs(wKingFile - bKingFile);

        // We don't want to negatively influence the eval so we add the constant to the distance
        constexpr uint8_t MAX_KING_DIST = 14;
        int kingDistTerm = kingDistMult * (MAX_KING_DIST - kingDist);

        int score = kingDistTerm + centerDistTerm;

        // If we have to rely on our bishop then we need to push to the correct corner
        if (matingPieceCounts[PieceType::Bishop] == 1 && matingPieceCounts[PieceType::Rook] == 0 && matingPieceCounts[PieceType::Queen] == 0)
        {
            bitboard bishops = whiteToWin ? board.getWhiteBishops() : board.getBlackBishops();
            square bishPos = bitBoards::firstSetBit(bishops);
            uint8_t bishRank = bishPos / 8;
            uint8_t bishFile = bishPos % 8;
            bool darkSquareBish = (bishFile + bishRank) % 2;
            // Give bonus for pushing king to a corner where the corner is of our bishops color
            uint8_t correctCornerDist = darkSquareBish ? tables::darkCornerDistance[losingKing]
                                                       : 7 - tables::darkCornerDistance[losingKing];

            constexpr int correctCornerMult = centerDistMult * 3 + kingDistMult * 3;
            score += correctCornerMult * (7 - correctCornerDist);
        }

        // make negative if black is winning
        return whiteToWin ? score : -score;
    }

    score Evaluator::evaluation()
    {
        score materialBalance = getMaterialBalance();

        // score for placement of the pieces
        float notEndGameNess = 1 - endGameNessScore;
        score positioningScore = endGameNessScore * endGameScore + notEndGameNess * middleGameScore;

        // kingsafety should weigh less in the endgame
        score kingSafetyScore = kingSafety() * notEndGameNess;

        // add score to encourage trading (non pawn) pieces when ahead
        // We use the piece percentage left to determine how much we should encourage trading
        constexpr float tradeEncouragementFactor = 0.2; // not experimentaly determined
        // We square it to make trades more wothit as less pieces are left
        float squaredPieceMaterialLeft = piecesMaterialLeft * piecesMaterialLeft;
        score tradeDownBonus = (1 - squaredPieceMaterialLeft) * materialBalance * tradeEncouragementFactor;

        // add a score to encourage driving the king to the corner
        float weight = mopUpFactor();
        score mopUpBonus = weight != 0 ? mopUpScore() * weight : 0;

        score eval = materialBalance + positioningScore + kingSafetyScore + tradeDownBonus + mopUpBonus;
        return eval;
    }
}