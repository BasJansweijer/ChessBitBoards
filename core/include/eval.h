#pragma once

#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"

constexpr int pieceVals[5] = {100, 300, 325, 500, 900};

namespace chess::engine
{
    int evaluate(BoardState b)
    {
        const chess::BoardState::PieceSet &white = b.getPieceSet(true);
        const chess::BoardState::PieceSet &black = b.getPieceSet(false);

        int eval = bitBoards::bitCount(white.pawns) * pieceVals[0] -
                   bitBoards::bitCount(black.pawns) * pieceVals[0] +
                   bitBoards::bitCount(white.knights) * pieceVals[1] -
                   bitBoards::bitCount(black.knights) * pieceVals[1] +
                   bitBoards::bitCount(white.bishops) * pieceVals[2] -
                   bitBoards::bitCount(black.bishops) * pieceVals[2] +
                   bitBoards::bitCount(white.rooks) * pieceVals[3] -
                   bitBoards::bitCount(black.rooks) * pieceVals[3] +
                   bitBoards::bitCount(white.queens) * pieceVals[4] -
                   bitBoards::bitCount(black.queens) * pieceVals[4];

        bitboard allPieces = white.allPieces | black.allPieces;

        bitBoards::forEachBit(white.bishops, [&](square s)
                              { eval += 5 * bitBoards::bitCount(constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(black.bishops, [&](square s)
                              { eval -= 5 * bitBoards::bitCount(constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(white.knights, [&](square s)
                              { eval += 10 * bitBoards::bitCount(constants::knightMoves[s]); });

        bitBoards::forEachBit(black.knights, [&](square s)
                              { eval -= 10 * bitBoards::bitCount(constants::knightMoves[s]); });

        bitBoards::forEachBit(white.rooks, [&](square s)
                              { eval += 7 * bitBoards::bitCount(constants::getRookMoves(s, allPieces)); });

        bitBoards::forEachBit(black.rooks, [&](square s)
                              { eval -= 7 * bitBoards::bitCount(constants::getRookMoves(s, allPieces)); });

        bitBoards::forEachBit(white.queens, [&](square s)
                              { eval += 3 * bitBoards::bitCount(constants::getRookMoves(s, allPieces) | constants::getBishopMoves(s, allPieces)); });

        bitBoards::forEachBit(black.queens, [&](square s)
                              { eval -= 3 * bitBoards::bitCount(constants::getRookMoves(s, allPieces) | constants::getBishopMoves(s, allPieces)); });

        return eval;
    }
}