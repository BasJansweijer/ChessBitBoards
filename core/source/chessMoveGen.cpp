#include "bitBoard.h"
#include "chess.h"
#include "boardVisualizer.h"
#include "moveConstants.h"
#include <iostream>
#include <vector>

namespace chess
{
    inline void addPromotionMove(square from, square to, bool wasCapture, std::vector<chess::Move> &outMoves)
    {
        outMoves.emplace_back(from, to, PieceType::Knight, wasCapture);
        outMoves.emplace_back(from, to, PieceType::Bishop, wasCapture);
        outMoves.emplace_back(from, to, PieceType::Rook, wasCapture);
        outMoves.emplace_back(from, to, PieceType::Queen, wasCapture);
    }

    // Loops through all the set squares of the bitboard and calls the callback with each set square
    inline void forEachSquare(bitboard bb, auto callback)
    {
        while (bb)
        {
            square pos = chess::bitBoards::firstSetBit(bb);
            callback(pos);
            bb ^= 1ULL << pos;
        }
    }

    inline void BoardState::addMoves(bitboard moves, square curPos, PieceType piece, std::vector<Move> &outMoves) const
    {
        forEachSquare(moves, [&](square moveTo)
                      {
            bool tookPiece = (allPieces(!m_whitesMove) & 1ULL << moveTo);
            outMoves.emplace_back(curPos, moveTo, piece, tookPiece); });
    }

    void BoardState::genKnightMoves(std::vector<Move> &outMoves) const
    {
        bitboard knights = m_whitesMove ? m_white.knights : m_black.knights;
        forEachSquare(knights, [&](square knightPos)
                      {
            bitboard moves = chess::constants::knightMoves[knightPos];
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, knightPos, PieceType::Knight, outMoves); });
    }

    void BoardState::genPawnMoves(std::vector<Move> &outMoves) const
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
                default: // Normal single forward step
                    outMoves.emplace_back(pawnPos, inFront, PieceType::Pawn, false);
                    break;
                }
            }

            // Pawn capture moves
            int file = pawnPos % 8;
            bitboard oppPieces = allPieces(!m_whitesMove) | 1ULL << m_enpassentSquare;

            if (file != 0)
            {
                // Check for capture to file-1;
                square leftTakes = pawnPos + (moveDir * 8) - 1;
                if (oppPieces & 1ULL << leftTakes && ranksMoved != 5)
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

    void BoardState::genKingMoves(std::vector<Move> &outMoves) const
    {
        bitboard kingBB = m_whitesMove ? m_white.king : m_black.king;
        square kingPos = bitBoards::firstSetBit(kingBB);
        bitboard moves = constants::kingMoves[kingPos];
        // remove self captures
        moves &= ~allPieces(m_whitesMove);

        addMoves(moves, kingPos, PieceType::King, outMoves);
    }

    void BoardState::genBishopMoves(std::vector<Move> &outMoves) const
    {
        bitboard bishops = m_whitesMove ? m_white.bishops : m_black.bishops;
        forEachSquare(bishops, [&](square bishopPos)
                      {
            bitboard moves = constants::getBishopMoves(bishopPos, allPieces());

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, bishopPos, PieceType::Bishop, outMoves); });
    }

    void BoardState::genRookMoves(std::vector<Move> &outMoves) const
    {
        bitboard rooks = m_whitesMove ? m_white.rooks : m_black.rooks;
        forEachSquare(rooks, [&](square rookPos)
                      {
            bitboard moves = constants::getRookMoves(rookPos, allPieces());

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, rookPos, PieceType::Rook, outMoves); });
    }

    void BoardState::genQueenMoves(std::vector<Move> &outMoves) const
    {
        bitboard queens = m_whitesMove ? m_white.queens : m_black.queens;
        forEachSquare(queens, [&](square queenPos)
                      {
            // Rook moves
            const constants::MagicInfo &rm = constants::rookMagics[queenPos];
            // compute the idx into the array by masking to get all blockers and using the magic
            int idx = rm.arrayOffset + (allPieces() & rm.mask) * rm.magic % rm.squareArraySize;
            bitboard moves = constants::rookNonBlockedMoves[idx];

            // Bishop moves
            const constants::MagicInfo &bm = constants::bishopMagics[queenPos];
            // compute the idx into the array by masking to get all blockers and using the magic
            idx = bm.arrayOffset + (allPieces() & bm.mask) * bm.magic % bm.squareArraySize;
            moves |= constants::bishopNonBlockedMoves[idx];

            // remove self captures
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, queenPos, PieceType::Queen, outMoves); });
    }

    std::vector<Move> BoardState::pseudoLegalMoves() const
    {
        std::vector<Move> moves;

        genPawnMoves(moves);
        genKnightMoves(moves);
        genBishopMoves(moves);
        genRookMoves(moves);
        genQueenMoves(moves);
        genKingMoves(moves);

        return moves;
    }

    std::vector<Move> BoardState::legalMoves() const
    {
        return pseudoLegalMoves();
    }
}