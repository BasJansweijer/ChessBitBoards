#include "eval.h"
#include "bitBoard.h"
#include "moveConstants.h"

constexpr int pieceVals[5] = {100, 300, 325, 500, 900};

namespace chess
{
    int evaluate(BoardState b)
    {

        const bitboard *white = b.getPieceSet(true);
        const bitboard *black = b.getPieceSet(false);

        int eval = bitBoards::bitCount(white[PieceType::Pawn]) * pieceVals[PieceType::Pawn] -
                   bitBoards::bitCount(black[PieceType::Pawn]) * pieceVals[PieceType::Pawn] +
                   bitBoards::bitCount(white[PieceType::Knight]) * pieceVals[PieceType::Knight] -
                   bitBoards::bitCount(black[PieceType::Knight]) * pieceVals[PieceType::Knight] +
                   bitBoards::bitCount(white[PieceType::Bishop]) * pieceVals[PieceType::Bishop] -
                   bitBoards::bitCount(black[PieceType::Bishop]) * pieceVals[PieceType::Bishop] +
                   bitBoards::bitCount(white[PieceType::Rook]) * pieceVals[PieceType::Rook] -
                   bitBoards::bitCount(black[PieceType::Rook]) * pieceVals[PieceType::Rook] +
                   bitBoards::bitCount(white[PieceType::Queen]) * pieceVals[PieceType::Queen] -
                   bitBoards::bitCount(black[PieceType::Queen]) * pieceVals[PieceType::Queen];

        bitboard allPieces = b.allPieces();

        bitBoards::forEachBit(white[PieceType::Bishop], [&](square s)
                              { eval += 5 * bitBoards::bitCount(constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(black[PieceType::Bishop], [&](square s)
                              { eval -= 5 * bitBoards::bitCount(constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(white[PieceType::Knight], [&](square s)
                              { eval += 10 * bitBoards::bitCount(constants::knightMoves[s]); });

        bitBoards::forEachBit(black[PieceType::Knight], [&](square s)
                              { eval -= 10 * bitBoards::bitCount(constants::knightMoves[s]); });

        bitBoards::forEachBit(white[PieceType::Rook], [&](square s)
                              { eval += 7 * bitBoards::bitCount(constants::getRookMoves(s, allPieces)); });

        bitBoards::forEachBit(black[PieceType::Rook], [&](square s)
                              { eval -= 7 * bitBoards::bitCount(constants::getRookMoves(s, allPieces)); });

        bitBoards::forEachBit(white[PieceType::Queen], [&](square s)
                              { eval += 3 * bitBoards::bitCount(constants::getRookMoves(s, allPieces) | constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(black[PieceType::Queen], [&](square s)
                              { eval -= 3 * bitBoards::bitCount(constants::getRookMoves(s, allPieces) | constants::getBishopMoves(s, allPieces)); });

        return eval;
    }
}