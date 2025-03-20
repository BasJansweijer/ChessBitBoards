#include "eval.h"
#include "bitBoard.h"
#include "moveConstants.h"
#include <cinttypes>
#include <algorithm>
#include "evalTables.h"

constexpr int pieceVals[5] = {100, 300, 320, 500, 900};
// The initial value of all (non pawn) pieces for a single player
using PT = chess::PieceType;
constexpr int startingPieceMaterial = pieceVals[PT::Knight] * 2 +
                                      pieceVals[PT::Bishop] * 2 +
                                      pieceVals[PT::Rook] * 2 +
                                      pieceVals[PT::Queen];

static uint8_t whitePieceCounts[5];
static uint8_t blackPieceCounts[5];
static float endGameNessScore;
static chess::BoardState position;

namespace tables = chess::evalTables;

namespace chess
{

    int kingSafety(const BoardState &board)
    {
        constexpr int KING_SAFETY_MULT = 7;
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
    float mopUpFactor(int whiteMaterial, int blackMaterial)
    {
        // If a player is up this much we might have to mop up.
        constexpr int minMaterialDiff = pieceVals[PieceType::Rook] - 2 * pieceVals[PieceType::Pawn];

        const int materialAdvantage = std::abs(whiteMaterial - blackMaterial);

        return materialAdvantage < minMaterialDiff || endGameNessScore < 0.5
                   ? 0 // don't use mopup score
                   : materialAdvantage * endGameNessScore / (float)pieceVals[PieceType::Rook];
    }

    /*
    Using the material of both players we determine a number in the range [0, 1]
    Where 0 means, this is not at all an endgame position and 1 means this is an endgame position.
    NOTE: we use "static" global pieceCount arrays for this
    */
    inline float isEndGameScore(int whiteMaterial, int blackMaterial)
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
        return scoreSquare * scoreSquare;
    }

    inline int mopUpScore(const BoardState &position, int materialBalance)
    {
        square whiteKing = position.getWhiteKingSquare();
        square blackKing = position.getBlackKingSquare();
        int8_t wKingRank = whiteKing / 8;
        int8_t wKingFile = whiteKing % 8;
        int8_t bKingRank = blackKing / 8;
        int8_t bKingFile = blackKing % 8;

        bool whiteToWin = materialBalance > 0;
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
            bitboard bishops = whiteToWin ? position.getWhiteBishops() : position.getBlackBishops();
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

    int evaluate(BoardState position)
    {

        const bitboard *white = position.getPieceSet(true);
        const bitboard *black = position.getPieceSet(false);

        // count the material for both sides
        int whiteMaterial = 0;
        int blackMaterial = 0;
        // Go through every piece (except king)
        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            whitePieceCounts[pieceType] = bitBoards::bitCount(white[pieceType]);
            whiteMaterial += whitePieceCounts[pieceType] * pieceVals[pieceType];
            blackPieceCounts[pieceType] = bitBoards::bitCount(black[pieceType]);
            blackMaterial += blackPieceCounts[pieceType] * pieceVals[pieceType];
        }
        endGameNessScore = isEndGameScore(whiteMaterial, blackMaterial);

        int middleGameScore = 0;
        int endGameScore = 0;
        // Go through every piece (except king)
        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            bitBoards::forEachBit(white[pieceType], [&](square s)
                                  { 
                middleGameScore += tables::middleGameWhite[pieceType][s];
                endGameScore += tables::endGameBlack[pieceType][s]; });

            bitBoards::forEachBit(black[pieceType], [&](square s)
                                  { 
                middleGameScore -= tables::middleGameBlack[pieceType][s];
                endGameScore -= tables::endGameBlack[pieceType][s]; });
        }

        // Add the king position score
        square whiteKing = position.getWhiteKingSquare();
        square blackKing = position.getBlackKingSquare();
        middleGameScore += tables::middleGameWhite[PieceType::King][whiteKing];
        middleGameScore -= tables::middleGameBlack[PieceType::King][blackKing];
        endGameScore += tables::endGameWhite[PieceType::King][whiteKing];
        endGameScore -= tables::endGameBlack[PieceType::King][blackKing];

        float notEndGameNess = 1 - endGameNessScore;

        int materialBalance = whiteMaterial - blackMaterial;

        // score for placement of the pieces
        int positioningScore = endGameNessScore * endGameScore + notEndGameNess * middleGameScore;

        int eval = materialBalance + positioningScore;

        // kingsafety should weigh less in the endgame
        eval += kingSafety(position) * notEndGameNess;

        // add a score to encourage driving the king to the corner
        float weight = mopUpFactor(whiteMaterial, blackMaterial);
        eval += weight != 0 ? mopUpScore(position, materialBalance) * weight : 0;

        return eval;
    }
}