#include "types.h"
#include "bitBoard.h"
#include "chess.h"
#include "moveConstants.h"
#include <vector>

namespace chess
{

    static bitboard s_movingPieces;
    static bitboard s_opponentPieces;
    static bitboard s_allPieces;

    using MoveGenType = BoardState::MoveGenType;

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

    template <MoveGenType GenT>
    inline void BoardState::addMoves(bitboard moves, square curPos, PieceType piece, MoveList &outMoves) const
    {
        // remove self captures (or for quiescent search only allow captures)
        GenT == MoveGenType::Normal ? moves &= ~s_movingPieces : moves &= s_opponentPieces;

        bitBoards::forEachBit(moves, [&](square moveTo)
                              {
            bool tookPiece = (s_opponentPieces & 1ULL << moveTo);
            outMoves.emplace_back(curPos, moveTo, piece, tookPiece); });
    }

    template <MoveGenType GenT>
    void BoardState::genKnightMoves(MoveList &outMoves) const
    {
        bitboard knights = m_whitesMove ? m_whitePieces[PieceType::Knight] : m_blackPieces[PieceType::Knight];
        bitBoards::forEachBit(knights, [&](square knightPos)
                              {
            bitboard moves = chess::constants::knightMoves[knightPos];
            addMoves<GenT>(moves, knightPos, PieceType::Knight, outMoves); });
    }

    template <MoveGenType GenT>
    void BoardState::genPawnMoves(MoveList &outMoves) const
    {
        uint8_t moveDir = m_whitesMove ? 1 : -1;
        bitboard pawns = m_whitesMove ? m_whitePieces[PieceType::Pawn] : m_blackPieces[PieceType::Pawn];

        bitBoards::forEachBit(pawns, [&](square pawnPos)
                              {
            int rank = pawnPos / 8;
            const int ranksMoved = m_whitesMove ? rank - 1 : -rank + 6;
            // Skip non capture moves in Quiescent moveGen
            if (GenT != MoveGenType::Quiescent)
            {
                // gives how many ranks the pawn has moved up/down (0 if not moved and 5 if on the rank before promotion)

                // generate normal step moves
                square inFront = pawnPos + moveDir * 8;
                const bitboard blockers = s_allPieces;
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
            }

            // Pawn capture moves
            int file = pawnPos % 8;
            // m_enpassent square larger than 64 has undefined behaviour.
            bitboard oppPieces = s_opponentPieces | ((m_enpassentSquare < 64) * 1ULL << m_enpassentSquare);

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

    template <MoveGenType GenT>
    void BoardState::genKingMoves(MoveList &outMoves) const
    {
        square kingPos = m_whitesMove ? m_whiteKing : m_blackKing;
        bitboard moves = constants::kingMoves[kingPos];
        addMoves<GenT>(moves, kingPos, PieceType::King, outMoves);
    }

    template <MoveGenType GenT>
    void BoardState::genBishopMoves(MoveList &outMoves) const
    {
        bitboard bishops = m_whitesMove ? m_whitePieces[PieceType::Bishop] : m_blackPieces[PieceType::Bishop];
        bitBoards::forEachBit(bishops, [&](square bishopPos)
                              {
            bitboard moves = constants::getBishopMoves(bishopPos, s_allPieces);
            addMoves<GenT>(moves, bishopPos, PieceType::Bishop, outMoves); });
    }

    template <MoveGenType GenT>
    void BoardState::genRookMoves(MoveList &outMoves) const
    {
        bitboard rooks = m_whitesMove ? m_whitePieces[PieceType::Rook] : m_blackPieces[PieceType::Rook];
        bitBoards::forEachBit(rooks, [&](square rookPos)
                              {
            bitboard moves = constants::getRookMoves(rookPos, s_allPieces);
            addMoves<GenT>(moves, rookPos, PieceType::Rook, outMoves); });
    }

    template <MoveGenType GenT>
    void BoardState::genQueenMoves(MoveList &outMoves) const
    {
        bitboard queens = m_whitesMove ? m_whitePieces[PieceType::Queen] : m_blackPieces[PieceType::Queen];
        bitBoards::forEachBit(queens, [&](square queenPos)
                              {
            bitboard blockers = s_allPieces;
            bitboard moves = constants::getRookMoves(queenPos, blockers);
            moves |= constants::getBishopMoves(queenPos, blockers);
            addMoves<GenT>(moves, queenPos, PieceType::Queen, outMoves); });
    }

    void BoardState::tryCastle(MoveList &outMoves, bool shortCastle) const
    {
        // Check wether the spots between the rook and the king are empty
        bitboard emptySpotMask = shortCastle ? 0b01100000 : 0b00001110;
        bitboard nonAttacked = shortCastle ? 0b01110000 : 0b0011100;
        const bitboard *movingPieces = m_whitesMove ? m_whitePieces : m_blackPieces;

        if (!m_whitesMove)
        {
            emptySpotMask <<= 8 * 7;
            nonAttacked <<= 8 * 7;
        }

        if (s_allPieces & emptySpotMask)
        {
            // pieces in the way
            return;
        }

        // Check wether the squares inbetween (or the king square) is attacked.
        while (nonAttacked)
        {
            square s = bitBoards::firstSetBit(nonAttacked);
            // remove the bit
            nonAttacked &= nonAttacked - 1;

            if (m_whitesMove ? squareAttacked<false>(s) : squareAttacked<true>(s))
                return;
        }

        square kingPos = m_whitesMove ? m_whiteKing : m_blackKing;
        square newKingPos = kingPos + (shortCastle ? 2 : -2);

        // We are allowed to castle.
        outMoves.emplace_back(kingPos, newKingPos, PieceType::King, false);
    }

    void BoardState::genCastlingMoves(MoveList &outMoves) const
    {
        if (m_whitesMove)
        {
            if (whiteCanCastleShort())
                tryCastle(outMoves, true);

            if (whiteCanCastleLong())
                tryCastle(outMoves, false);
        }
        else
        {
            if (blackCanCastleShort())
                tryCastle(outMoves, true);

            if (blackCanCastleLong())
                tryCastle(outMoves, false);
        }
    }

    template <MoveGenType GenType>
    MoveList BoardState::pseudoLegalMoves() const
    {
        MoveList moves;

        if (m_whitesMove)
        {
            s_movingPieces = whitePieces();
            s_opponentPieces = blackPieces();
        }
        else
        {
            s_movingPieces = blackPieces();
            s_opponentPieces = whitePieces();
        }

        s_allPieces = s_movingPieces | s_opponentPieces;

        genPawnMoves<GenType>(moves);
        genKnightMoves<GenType>(moves);
        genBishopMoves<GenType>(moves);
        genRookMoves<GenType>(moves);
        genQueenMoves<GenType>(moves);
        genKingMoves<GenType>(moves);

        // Castling can't be a capture
        if (GenType == MoveGenType::Normal)
            genCastlingMoves(moves);

        return moves;
    }

    MoveList BoardState::legalMoves() const
    {
        MoveList legal;
        for (auto &m : pseudoLegalMoves<MoveGenType::Normal>())
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

    // Template instantiations
    template MoveList BoardState::pseudoLegalMoves<MoveGenType::Normal>() const;
    template MoveList BoardState::pseudoLegalMoves<MoveGenType::Quiescent>() const;
}