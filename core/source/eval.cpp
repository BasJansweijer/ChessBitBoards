#include "eval.h"
#include "bitBoard.h"
#include "moveConstants.h"
#include <cinttypes>
#include "evalTables.h"

constexpr int pieceVals[5] = {100, 300, 320, 500, 900};

static uint8_t whitePieceCounts[5];
static uint8_t blackPieceCounts[5];

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

    // Calculates a float [0-1] which determines how much the endgame evaluation should weigh
    // compared to the normal evaluation tables
    float endGameFactor()
    {
        return 0.0;
    }

    // mopup score strongly influences the normal evaluation at the very end of the game to promote cornering the losing king
    bool useMopUpScore(int materialBalance)
    {
        return materialBalance > 0 ? blackPieceCounts[PieceType::Pawn] == 0 : whitePieceCounts[PieceType::Pawn] == 0;
    }

    int mopUpScore(int materialBalance, square whiteKing, square blackKing)
    {
        int8_t wKingRank = whiteKing / 8;
        int8_t wKingFile = whiteKing % 8;
        int8_t bKingRank = blackKing / 8;
        int8_t bKingFile = blackKing % 8;

        bool whiteToWin = materialBalance > 0;

        constexpr int centerDistMult = 400;
        constexpr int kingDistMult = 100;

        int centerDistTerm = tables::centerManhattanDistance[whiteToWin ? blackKing : whiteKing] * centerDistMult;
        uint8_t kingDist = abs(wKingRank - bKingRank) + abs(wKingFile - bKingFile);

        // We don't want to negatively influence the eval so we add the constant to the distance
        constexpr uint8_t MAX_KING_DIST = 14;
        int kingDistTerm = kingDistMult * (MAX_KING_DIST - kingDist);

        return kingDistTerm + centerDistTerm;
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

        int middelGameScore = 0;
        int endGameScore = 0;
        // Go through every piece (except king)
        for (int pieceType = 0; pieceType < 5; pieceType++)
        {
            bitBoards::forEachBit(white[pieceType], [&](square s)
                                  { middelGameScore += tables::middleGameWhite[pieceType][s]; });

            bitBoards::forEachBit(black[pieceType], [&](square s)
                                  { middelGameScore -= tables::middleGameBlack[pieceType][s]; });
        }

        // Add the king position score
        square whiteKing = position.getWhiteKingSquare();
        square blackKing = position.getBlackKingSquare();
        middelGameScore += tables::middleGameWhite[PieceType::King][whiteKing];
        middelGameScore -= tables::middleGameBlack[PieceType::King][blackKing];

        int materialBalance = whiteMaterial - blackMaterial;

        int eval = materialBalance + middelGameScore;

        eval += kingSafety(position);

        // If there are no more pawns we force losing king to the corner using the mopUpScore
        bool useMopUp = useMopUpScore(materialBalance);
        eval += useMopUp ? mopUpScore(materialBalance, whiteKing, blackKing) : 0;

        return eval;
    }
}