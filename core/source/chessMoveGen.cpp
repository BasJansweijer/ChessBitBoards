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

    inline void BoardState::addMoves(bitboard moves, square curPos, PieceType piece, std::vector<Move> &outMoves) const
    {
        while (moves)
        {
            square moveTo = chess::bitBoards::firstSetBit(moves);

            bool tookPiece = (allPieces(!m_whitesMove) & 1ULL << moveTo);

            outMoves.emplace_back(curPos, moveTo, piece, tookPiece);

            // remove this move
            moves ^= 1ULL << moveTo;
        }
    }

    void BoardState::genKnightMoves(std::vector<Move> &outMoves) const
    {
        bitboard knights = m_whitesMove ? m_whiteKnights : m_blackKnights;
        while (knights)
        {
            square knightPos = chess::bitBoards::firstSetBit(knights);

            // remove this knight position
            knights ^= 1ULL << knightPos;

            bitboard moves = chess::constants::knightMoves[knightPos];
            moves &= ~allPieces(m_whitesMove);

            addMoves(moves, knightPos, PieceType::Knight, outMoves);
        }
    }

    void BoardState::genPawnMoves(std::vector<Move> &outMoves) const
    {
        uint8_t moveDir = m_whitesMove ? 1 : -1;
        bitboard pawns = m_whitesMove ? m_whitePawns : m_blackPawns;

        while (pawns)
        {
            square pawnPos = chess::bitBoards::firstSetBit(pawns);
            // gives how many ranks the pawn has moved up/down (0 if not moved and 5 if on the rank before promotion)
            int rank = pawnPos / 8;
            const int ranksMoved = m_whitesMove ? rank - 1 : -rank + 6;

            // remove this pawn position
            pawns ^= 1ULL << pawnPos;

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
            }
        }
    }

    void BoardState::genKingMoves(std::vector<Move> &outMoves) const
    {
        bitboard kingBB = m_whitesMove ? m_whiteKing : m_blackKing;
        square kingPos = bitBoards::firstSetBit(kingBB);
        bitboard moves = constants::kingMoves[kingPos];
        // remove self captures
        moves &= ~allPieces(m_whitesMove);

        addMoves(moves, kingPos, PieceType::King, outMoves);
    }

    std::vector<Move> BoardState::pseudoLegalMoves() const
    {
        std::vector<Move> moves;

        genPawnMoves(moves);
        genKnightMoves(moves);
        genKingMoves(moves);

        return moves;
    }

    std::vector<Move> BoardState::legalMoves() const
    {
        return pseudoLegalMoves();
    }
}