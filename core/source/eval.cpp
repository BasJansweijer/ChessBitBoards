#include "eval.h"
#include "bitBoard.h"
#include "moveConstants.h"
#include <cinttypes>
#include <algorithm>
#include "evalTables.h"
#include "masks.h"

namespace tables = chess::evalTables;

namespace chess
{
    template <bool isWhite>
    score Evaluator::kingSafety()
    {
        square king = isWhite ? m_board.getWhiteKingSquare() : m_board.getBlackKingSquare();
        constexpr bitboard backRank = mask::rankMask(isWhite ? 0 : 7);

        bitboard ourPawns = isWhite ? m_board.getWhitePawns() : m_board.getBlackPawns();
        // act as if there is a queen on the kins position
        bitboard virtualMoves = constants::getBishopMoves(king, ourPawns) | constants::getRookMoves(king, ourPawns);
        // remove our pawns and the backrank from counting as open
        virtualMoves &= ~(ourPawns & backRank);

        constexpr int KING_SAFETY_MULT = 10;
        score safetyScore = -(bitBoards::bitCount(virtualMoves) * KING_SAFETY_MULT);

        return safetyScore;
    }

    // mopup score strongly influences the normal evaluation at the very end of the game to promote cornering the losing king
    float Evaluator::mopUpFactor()
    {
        // If a player is up this much we might have to mop up.
        constexpr int minMaterialDiff = pieceVals[PieceType::Rook] - 2 * pieceVals[PieceType::Pawn];

        const int materialAdvantage = std::abs(getMaterialBalance());

        // thee factor is lower when opponent has more (non pawn) material still on the board
        const int opponentMaterial = m_whitePieceMaterial > m_blackPieceMaterial ? m_blackPieceMaterial : m_whitePieceMaterial;

        return materialAdvantage < minMaterialDiff || m_endGameNessScore < 0.5
                   ? 0 // don't use mopup score
                   : materialAdvantage * m_endGameNessScore / (float)(pieceVals[PieceType::Rook] + opponentMaterial);
    }

    score Evaluator::mopUpScore()
    {
        square whiteKing = m_board.getWhiteKingSquare();
        square blackKing = m_board.getBlackKingSquare();
        int8_t wKingRank = whiteKing / 8;
        int8_t wKingFile = whiteKing % 8;
        int8_t bKingRank = blackKing / 8;
        int8_t bKingFile = blackKing % 8;

        bool whiteToWin = getMaterialBalance() > 0;
        uint8_t *matingPieceCounts = whiteToWin ? m_whitePieceCounts : m_blackPieceCounts;

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
            bitboard bishops = whiteToWin ? m_board.getWhiteBishops() : m_board.getBlackBishops();
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

    template <bool isWhite>
    constexpr score passedPawnBonus(uint8_t rank, float endgameNessScore)
    {
        constexpr score baseBonus = 30; // small as we also give bonus per file
        // There is already some bonus in the piece square tables but we give a little extra
        constexpr score rankBonusMult = 3;

        uint8_t movedRanks = isWhite ? rank - 1 : 6 - rank;
        // Make rankBonus non linear (movedRanks^2 gives [0,1,4,9,16,25,36])
        score rankBonus = rankBonusMult * (movedRanks * movedRanks);

        // endgameNess score is between 0 and 1
        float endgameNessMult = endgameNessScore + 0.2; // always give at least 0.2 of the rankBonus
        return baseBonus + rankBonus * endgameNessMult;
    }

    template <bool isWhite>
    score Evaluator::pawnStructureAnalysis()
    {
        bitboard ourPawns = isWhite ? m_whiteBitBoards[PieceType::Pawn] : m_blackBitBoards[PieceType::Pawn];
        bitboard oppPawns = isWhite ? m_blackBitBoards[PieceType::Pawn] : m_whiteBitBoards[PieceType::Pawn];

        constexpr score isolationPenalty = 15;
        constexpr score defendedPawnBonus = 5;
        // We want to also multiply the value of the pawn when defended to
        // prioritize defending valuable pawns
        constexpr float defendedPawnMult = 1.1;

        score structureScore = 0;

        // Lambda to analyze a single pawn
        auto pawnAnalysis = [&](square s)
        {
            uint8_t file = s % 8;
            uint8_t rank = s / 8;

            // isolatedPawn analysis
            bool hasLeftNeighbor = file > 0 && containsPawn<isWhite>(m_fileTypes[file - 1]);
            bool hasRightNeighbor = file < 7 && containsPawn<isWhite>(m_fileTypes[file + 1]);
            bool isIsolated = !(hasLeftNeighbor || hasRightNeighbor);

            score pawnScore = 0;
            if (isIsolated)
                pawnScore -= isolationPenalty;

            // passedPawn analysis
            bitboard mask = mask::passedPawn<isWhite>(s);
            // check if there are any opponent pawns in the mask
            bool isPassedPawn = (mask & oppPawns) == 0;
            if (isPassedPawn)
                pawnScore += passedPawnBonus<isWhite>(rank, m_endGameNessScore);

            // Check if were defended
            bool isDefended = (mask::pawnAttack<!isWhite>(s) & ourPawns) != 0;
            if (isDefended)
            {
                pawnScore += defendedPawnBonus; // small bonus for being defended
                // We also multiply to give extra value to defending valuable pawns (passed pawns)
                pawnScore *= defendedPawnMult;
            }

            // Add this pawns score to the total
            structureScore += pawnScore;
        };

        // Do the analysis for each white/black pawn
        bitBoards::forEachBit(ourPawns, pawnAnalysis);

        return structureScore;
    }

    template <bool isWhite>
    score Evaluator::bishopPairBonus()
    {
        constexpr score bishopPairBonus = 30;
        uint8_t *ourPieceCounts = isWhite ? m_whitePieceCounts : m_blackPieceCounts;
        return ourPieceCounts[PieceType::Bishop] >= 2 ? bishopPairBonus : 0;
    }

    // Score is used when endgameness > 0.9
    // gives penalty for the distance to the nearest (friendly/enemy) pawn.
    // gives penalty for passers that we cannot catch (especially when no other pieces are left)
    template <bool isWhite>
    score Evaluator::kingPositionScore()
    {
        if (m_endGameNessScore <= 0.9)
            return 0; // not yet important

        bitboard ourPawns = isWhite ? m_whiteBitBoards[Pawn] : m_blackBitBoards[Pawn];
        bitboard enemyPawns = isWhite ? m_blackBitBoards[Pawn] : m_whiteBitBoards[Pawn];

        bitboard pawns = ourPawns | enemyPawns;
        if (pawns == 0)
            return 0; // no pawns left to measure distance to

        // normalize weight between [0-1]
        float weight = 10 * (m_endGameNessScore - 0.9);

        square king = isWhite ? m_board.getWhiteKingSquare() : m_board.getBlackKingSquare();
        bitboard kingMask = 1ULL << king;

        uint8_t dist = 0;
        bitboard area = pawns;
        while ((area & kingMask) == 0)
        {
            dist++;
            // Increase mask to include the next step
            area = mask::oneStep(area);
        }

        constexpr score kingDistPenalty = 5;
        // dist - 1 since distance will always be atleast 1
        score kingPosScore = -kingDistPenalty * (dist - 1);

        // mask to check if the king is in "the square"
        bitboard kingLocationMask = m_board.whitesMove() != isWhite
                                        ? kingMask                    // passed pawn moves first
                                        : constants::kingMoves[king]; // king moves first

        constexpr score notInSquarePenalty = 20;
        // If there are no pieces left besides the king and the passed pawn
        // then we know that the pawn can 100% be promoted
        constexpr score unCatchablePenalty = 300;

        uint8_t *ourPieceCounts = isWhite ? m_whitePieceCounts : m_blackPieceCounts;
        // used to determine if we have pieces left to catch the passed pawn if the king can't catch it
        bool weHavePieces = ourPieceCounts[PieceType::Rook] > 0 || ourPieceCounts[PieceType::Queen] > 0 ||
                            ourPieceCounts[PieceType::Knight] > 0 || ourPieceCounts[PieceType::Bishop] > 0;

        bitBoards::forEachBit(enemyPawns,
                              [&](square s)
                              {
            // Check if the pawn is passed
            bitboard mask = mask::passedPawn<!isWhite>(s);
            bool isPassedPawn = (mask & ourPawns) == 0;
            if (!isPassedPawn)
                return;
            // check if we are 'in the square' of the passed pawn
            bool inSquare = (kingLocationMask & mask::pawnSquare<!isWhite>(s)) != 0;
            if (inSquare)
                return; // no penalty as the king can catch the pawn
            score penalty = weHavePieces ? notInSquarePenalty : unCatchablePenalty;
            kingPosScore -= penalty; });

        return weight * kingPosScore;
    }

    template <bool isWhite>
    score Evaluator::rookOpenFileBonus()
    {
        constexpr int openFileBonus = 20;
        constexpr int halfOpenFileBonus = 10;

        constexpr FileType halfOpen = isWhite ? FileType::HALF_OPEN_WHITE : FileType::HALF_OPEN_BLACK;

        score bonusses = 0;
        auto openFileAnalysis = [&](square s)
        {
            uint8_t file = s % 8;
            if (m_fileTypes[file] == FileType::OPEN)
                bonusses += openFileBonus;
            else if (m_fileTypes[file] == halfOpen)
                bonusses += halfOpenFileBonus;
        };

        bitBoards::forEachBit(isWhite ? m_whiteBitBoards[PieceType::Rook] : m_blackBitBoards[PieceType::Rook],
                              openFileAnalysis);

        return bonusses;
    }

    score Evaluator::evaluation()
    {
        score materialBalance = getMaterialBalance();
        score eval = materialBalance;

        // score for placement of the pieces
        float notEndGameNess = 1 - m_endGameNessScore;
        score positioningScore = m_endGameNessScore * m_endGameScore + notEndGameNess * m_middleGameScore;
        eval += positioningScore;

        // kingsafety is scaled internally by amount of pieces left of enemy
        score whiteSafetyScore = kingSafety<true>();
        score blackSafetyScore = kingSafety<false>();
        eval += (whiteSafetyScore - blackSafetyScore) * notEndGameNess;

        // pawn structure analysis
        eval += pawnStructureAnalysis<true>();  // white
        eval -= pawnStructureAnalysis<false>(); // black

        eval += kingPositionScore<true>();
        eval -= kingPositionScore<false>();

        // rook open file bonus
        eval += rookOpenFileBonus<true>();  // white bonusses
        eval -= rookOpenFileBonus<false>(); // black bonusses

        // bishop pair bonus
        eval += bishopPairBonus<true>();
        eval -= bishopPairBonus<false>();

        // add score to encourage trading (non pawn) pieces when ahead
        // We use the piece percentage left to determine how much we should encourage trading
        constexpr float tradeEncouragementFactor = 0.2; // not experimentaly determined
        // We square it to make trades more wothit as less pieces are left
        float squaredPieceMaterialLeft = m_piecesMaterialLeft * m_piecesMaterialLeft;
        score tradeDownBonus = (1 - squaredPieceMaterialLeft) * materialBalance * tradeEncouragementFactor;
        eval += tradeDownBonus;

        // add a score to encourage driving the king to the corner
        float weight = mopUpFactor();
        score mopUpBonus = weight != 0 ? mopUpScore() * weight : 0;
        eval += mopUpBonus;

        return eval;
    }
}