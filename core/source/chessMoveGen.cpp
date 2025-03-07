#include "bitBoard.h"
#include "chess.h"
#include "moveConstants.h"
#include <vector>

namespace chess
{
    inline void addPromotionMove(square from, square to, bool wasCapture, MoveList &outMoves)
    {
        outMoves.emplace_back(from, to, PieceType::Knight, wasCapture);
        outMoves.back().promotion = true;
        outMoves.emplace_back(from, to, PieceType::Bishop, wasCapture);
        outMoves.back().promotion = true;
        outMoves.emplace_back(from, to, PieceType::Rook, wasCapture);
        outMoves.back().promotion = true;
        outMoves.emplace_back(from, to, PieceType::Queen, wasCapture);
        outMoves.back().promotion = true;
    }

    // Loops through all the set squares of the bitboard and calls the callback with each set square
    inline void forEachSquare(bitboard bb, auto callback)
    {
        while (bb)
        {
            square pos = chess::bitBoards::popFirstBit(bb);
            callback(pos);
        }
    }

    inline void BoardState::addMoves(bitboard moves, square curPos, PieceType piece, MoveList &outMoves) const
    {
        forEachSquare(moves, [&](square moveTo)
                      {
            bool tookPiece = (allPieces(!m_whitesMove) & 1ULL << moveTo);
            outMoves.emplace_back(curPos, moveTo, piece, tookPiece); });
    }

    void BoardState::genKnightMoves(MoveList &outMoves) const
    {
        bitboard knights = m_whitesMove ? m_white.knights : m_black.knights;
        forEachSquare(knights, [&](square knightPos)
                      {
            bitboard moves = chess::constants::knightMoves[knightPos];
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, knightPos, PieceType::Knight, outMoves); });
    }

    void BoardState::genPawnMoves(MoveList &outMoves) const
    {
        uint8_t moveDir = m_whitesMove ? 1 : -1;
        bitboard pawns = m_whitesMove ? m_white.pawns : m_black.pawns;

        forEachSquare(pawns, [&](square pawnPos)
                      {
            // gives how many ranks the pawn has moved up/down (0 if not moved and 5 if on the rank before promotion)
            int rank = pawnPos / 8;
            const int ranksMoved = m_whitesMove ? rank - 1 : -rank + 6;

            // generate normal step moves
            square inFront = pawnPos + moveDir * 8;
            bitboard blockers = allPieces();
            bool blocked = blockers & 1ULL << inFront;
            if (!blocked)
            {
                switch (ranksMoved)
                {
                case 0:
                {
                    // on starting square
                    outMoves.emplace_back(pawnPos, inFront, PieceType::Pawn, false);
                    square twoInFront = pawnPos + moveDir * 16;
                    if (!(blockers & 1ULL << twoInFront))
                        outMoves.emplace_back(pawnPos, twoInFront, PieceType::Pawn, false);
                    break;
                }
                case 5: // Promotions
                    addPromotionMove(pawnPos, inFront, false, outMoves);
                    break;
                default: // Normal single forward step
                    outMoves.emplace_back(pawnPos, inFront, PieceType::Pawn, false);
                    break;
                }
            }

            // Pawn capture moves
            int file = pawnPos % 8;
            // m_enpassent square larger than 64 has undefined behaviour.
            bitboard oppPieces = allPieces(!m_whitesMove) | ((m_enpassentSquare < 64) * 1ULL << m_enpassentSquare);

            if (file != 0)
            {
                // Check for capture to file-1;
                square leftTakes = pawnPos + (moveDir * 8) - 1;
                bool canCapture = oppPieces & 1ULL << leftTakes;
                if (canCapture)
                {
                    if (ranksMoved == 5)
                        addPromotionMove(pawnPos, leftTakes, true, outMoves);
                    else // the default pawn capture
                        outMoves.emplace_back(pawnPos, leftTakes, PieceType::Pawn, true);
                }
            }

            if (file != 7)
            {
                // Check for capture to file+1;
                square rightTakes = pawnPos + (moveDir * 8) + 1;
                bool canCapture = oppPieces & 1ULL << rightTakes;
                if (canCapture)
                {
                    if (ranksMoved == 5)
                        addPromotionMove(pawnPos, rightTakes, true, outMoves);
                    else // the default pawn capture
                        outMoves.emplace_back(pawnPos, rightTakes, PieceType::Pawn, true);
                }
            } });
    }

    void BoardState::genKingMoves(MoveList &outMoves) const
    {
        square kingPos = m_whitesMove ? m_white.king : m_black.king;
        bitboard moves = constants::kingMoves[kingPos];
        // remove self captures
        moves &= ~allPieces(m_whitesMove);

        addMoves(moves, kingPos, PieceType::King, outMoves);
    }

    void BoardState::genBishopMoves(MoveList &outMoves) const
    {
        bitboard bishops = m_whitesMove ? m_white.bishops : m_black.bishops;
        forEachSquare(bishops, [&](square bishopPos)
                      {
            bitboard moves = constants::getBishopMoves(bishopPos, allPieces());

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, bishopPos, PieceType::Bishop, outMoves); });
    }

    void BoardState::genRookMoves(MoveList &outMoves) const
    {
        bitboard rooks = m_whitesMove ? m_white.rooks : m_black.rooks;
        forEachSquare(rooks, [&](square rookPos)
                      {
            bitboard moves = constants::getRookMoves(rookPos, allPieces());

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, rookPos, PieceType::Rook, outMoves); });
    }

    void BoardState::genQueenMoves(MoveList &outMoves) const
    {
        bitboard queens = m_whitesMove ? m_white.queens : m_black.queens;
        forEachSquare(queens, [&](square queenPos)
                      {
            bitboard blockers = allPieces();
            bitboard moves = constants::getRookMoves(queenPos, blockers);
            moves |= constants::getBishopMoves(queenPos, blockers);

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, queenPos, PieceType::Queen, outMoves); });
    }

    void BoardState::tryCastle(MoveList &outMoves, bool shortCastle) const
    {
        // Check wether the spots between the rook and the king are empty
        bitboard emptySpotMask = shortCastle ? 0b01100000 : 0b00001110;
        bitboard nonAttacked = shortCastle ? 0b01110000 : 0b0011100;
        const PieceSet &movingPieces = m_whitesMove ? m_white : m_black;
        if (!m_whitesMove)
        {
            emptySpotMask <<= 8 * 7;
            nonAttacked <<= 8 * 7;
        }

        if (allPieces() & emptySpotMask)
        {
            // pieces in the way
            return;
        }

        // Check wether the squares inbetween (or the king square) is attacked.
        while (nonAttacked)
        {
            square s = bitBoards::popFirstBit(nonAttacked);
            if (m_whitesMove ? squareAttacked<false>(s) : squareAttacked<true>(s))
                return;
        }

        square newKingPos = movingPieces.king + (shortCastle ? 2 : -2);

        // We are allowed to castle.
        outMoves.emplace_back(movingPieces.king, newKingPos, PieceType::King, false);
    }

    void BoardState::genCastlingMoves(MoveList &outMoves) const
    {
        if (m_whitesMove)
        {
            if (m_whiteCanCastleShort)
                tryCastle(outMoves, true);

            if (m_whiteCanCastleLong)
                tryCastle(outMoves, false);
        }
        else
        {
            if (m_blackCanCastleShort)
                tryCastle(outMoves, true);

            if (m_blackCanCastleLong)
                tryCastle(outMoves, false);
        }
    }

    MoveList BoardState::pseudoLegalMoves() const
    {
        MoveList moves;

        genPawnMoves(moves);
        genKnightMoves(moves);
        genBishopMoves(moves);
        genRookMoves(moves);
        genQueenMoves(moves);
        genKingMoves(moves);
        genCastlingMoves(moves);

        return moves;
    }

    MoveList BoardState::legalMoves() const
    {
        MoveList legal;
        for (auto &m : pseudoLegalMoves())
        {
            chess::BoardState bNew = *this;
            bNew.makeMove(m);

            // if king is under attack on opponents move thats illegal
            if (bNew.kingAttacked(!bNew.whitesMove()))
                continue;

            legal.push_back(m);
        }

        return legal;
    }
}