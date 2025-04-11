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
        square blackKing = m_board.getBlackKingSquare();
        square whiteKing = m_board.getWhiteKingSquare();

        bitboard whitePawns = m_board.getWhitePawns();
        bitboard blackPawns = m_board.getBlackPawns();

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

        const int materialAdvantage = std::abs(m_whiteMaterial - m_blackMaterial);

        // We factor is lower when opponent has more material still on the board
        const int opponentMaterial = m_whiteMaterial > m_blackMaterial ? m_blackMaterial : m_whiteMaterial;

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
    constexpr bitboard passedPawnMask(square s)
    {
        uint8_t rank = s / 8;
        uint8_t file = s % 8;

        bitboard mask = 0;
        // Three files that can stop the pawn
        mask |= bitBoards::fileMask(file);
        mask |= file > 0 ? bitBoards::fileMask(file - 1) : 0;
        mask |= file < 7 ? bitBoards::fileMask(file + 1) : 0;

        // Shift up or down depending on the color
        uint8_t shift = 8 * (rank + (isWhite ? 1 : 0));
        isWhite ? mask <<= shift : mask >>= -shift;
        return mask;
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
        score isolatedPawnPenalties = 0;
        score passedPawnBonuses = 0;

        // Lambda to analyze a single pawn
        auto pawnAnalysis = [&](square s)
        {
            uint8_t file = s % 8;
            uint8_t rank = s / 8;

            // isolatedPawn analysis
            bool hasLeftNeighbor = file > 0 && containsPawn<isWhite>(m_fileTypes[file - 1]);
            bool hasRightNeighbor = file < 7 && containsPawn<isWhite>(m_fileTypes[file + 1]);
            bool isIsolated = !(hasLeftNeighbor || hasRightNeighbor);
            if (isIsolated)
                isolatedPawnPenalties -= isolationPenalty;

            // passedPawn analysis
            bitboard mask = passedPawnMask<isWhite>(s);
            // check if there are any opponent pawns in the mask
            bool isPassedPawn = (mask & oppPawns) == 0;
            if (isPassedPawn)
                passedPawnBonuses += passedPawnBonus<isWhite>(rank, m_endGameNessScore);
        };

        // Do the analysis for each white/black pawn
        bitBoards::forEachBit(ourPawns, pawnAnalysis);

        // TODO: Isolated pawn penalties currently not included
        return passedPawnBonuses;
    }

    score Evaluator::evaluation()
    {
        score materialBalance = getMaterialBalance();
        score eval = materialBalance;

        // score for placement of the pieces
        float notEndGameNess = 1 - m_endGameNessScore;
        score positioningScore = m_endGameNessScore * m_endGameScore + notEndGameNess * m_middleGameScore;
        eval += positioningScore;

        // kingsafety should weigh less in the endgame
        score kingSafetyScore = kingSafety() * notEndGameNess;
        eval += kingSafetyScore;

        // pawn structure analysis
        score whitePawnStructureScore = pawnStructureAnalysis<true>();
        score blackPawnStructureScore = pawnStructureAnalysis<false>();
        eval += whitePawnStructureScore;
        eval -= blackPawnStructureScore;

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