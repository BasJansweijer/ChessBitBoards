
#include <stdexcept>
#include "chess.h"
#include "bitBoard.h"
#include "moveConstants.h"
#include "zobristHash.h"

namespace chess
{
    template <bool ByWhite>
    bool BoardState::squareAttacked(square s) const
    {
        // bitboards of all the pieces that could attack the square
        const bitboard *attacker = ByWhite ? m_whitePieces : m_blackPieces;
        const square attackerKing = ByWhite ? m_whiteKing : m_blackKing;

        bitboard allBlockers = allPieces();
        int8_t file = s % 8;

        // compute the square above/bellow square s (on rank from which pawns can attack)
        square pawnRank = s - (ByWhite ? 8 : -8);
        square pawnAttackRight = pawnRank + 1;
        square pawnAttackLeft = pawnRank - 1;

        // We check if we aren't on the edge of the board since in that case
        // the pawn attack square doesn't exist
        return (file != 0 && attacker[PieceType::Pawn] & 1ULL << pawnAttackLeft) ||
               (file != 7 && attacker[PieceType::Pawn] & 1ULL << pawnAttackRight) ||
               constants::knightMoves[s] & attacker[PieceType::Knight] ||
               constants::kingMoves[s] & 1ULL << attackerKing ||
               constants::getBishopMoves(s, allBlockers) & (attacker[PieceType::Bishop] | attacker[PieceType::Queen]) ||
               constants::getRookMoves(s, allBlockers) & (attacker[PieceType::Rook] | attacker[PieceType::Queen]);
    }

    template bool BoardState::squareAttacked<true>(square s) const;
    template bool BoardState::squareAttacked<false>(square s) const;

    bool BoardState::kingAttacked(bool white) const
    {
        return white ? squareAttacked<false>(m_whiteKing) : squareAttacked<true>(m_blackKing);
    }

    template <PieceType piece, bool white>
    void BoardState::movePiece(square from, square to)
    {
        if constexpr (piece == PieceType::King)
        {
            square &king = white ? m_whiteKing : m_blackKing;
            king = to;
        }
        else
        {
            bitboard &effectedBitboard = white ? m_whitePieces[piece] : m_blackPieces[piece];
            effectedBitboard ^= 1ULL << to | 1ULL << from;
        }

        // update hash
        // constexpr int pieceIdx = piece + white ? 0 : 6;
        // m_hash ^= zobrist::squarePieceKeys[from][pieceIdx];
        // m_hash ^= zobrist::squarePieceKeys[to][pieceIdx];
    }

    template void BoardState::movePiece<PieceType::Pawn, false>(square from, square to);
    template void BoardState::movePiece<PieceType::Knight, false>(square from, square to);
    template void BoardState::movePiece<PieceType::Bishop, false>(square from, square to);
    template void BoardState::movePiece<PieceType::Rook, false>(square from, square to);
    template void BoardState::movePiece<PieceType::Queen, false>(square from, square to);
    template void BoardState::movePiece<PieceType::King, false>(square from, square to);

    template void BoardState::movePiece<PieceType::Pawn, true>(square from, square to);
    template void BoardState::movePiece<PieceType::Knight, true>(square from, square to);
    template void BoardState::movePiece<PieceType::Bishop, true>(square from, square to);
    template void BoardState::movePiece<PieceType::Rook, true>(square from, square to);
    template void BoardState::movePiece<PieceType::Queen, true>(square from, square to);
    template void BoardState::movePiece<PieceType::King, true>(square from, square to);

    template <PieceType piece, bool white>
    void BoardState::togglePiece(square s)
    {
        if constexpr (piece == PieceType::King)
        {
            // Should not really be used in general since we can't have multiple instances of the king
            throw std::runtime_error("togglePiece should not be called for the King. Use movePiece instead.");
        }

        bitboard &effectedBitboard = white ? m_whitePieces[piece] : m_blackPieces[piece];
        effectedBitboard ^= 1ULL << s;

        // update hash
        // constexpr int pieceIdx = piece + white ? 0 : 6;
        // m_hash ^= zobrist::squarePieceKeys[s][pieceIdx];
    }

    template <bool white>
    void BoardState::togglePiece(PieceType piece, square s)
    {
        switch (piece)
        {
        case Pawn:
            togglePiece<Pawn, white>(s);
            break;
        case Knight:
            togglePiece<Knight, white>(s);
            break;
        case Bishop:
            togglePiece<Bishop, white>(s);
            break;
        case Rook:
            togglePiece<Rook, white>(s);
            break;
        case Queen:
            togglePiece<Queen, white>(s);
            break;
        case King:
            togglePiece<King, white>(s);
            break;
        }
    }

    template void BoardState::togglePiece<true>(PieceType piece, square s);
    template void BoardState::togglePiece<false>(PieceType piece, square s);

    template <bool white>
    PieceType BoardState::pieceOnSquare(square s) const
    {
        const bitboard *pieces = white ? m_whitePieces : m_blackPieces;
        bitboard location = 1ULL << s;

        if (pieces[Pawn] & location)
            return Pawn;
        else if (pieces[Knight] & location)
            return Knight;
        else if (pieces[Bishop] & location)
            return Bishop;
        else if (pieces[Rook] & location)
            return Rook;
        else if (pieces[Queen] & location)
            return Queen;

        square king = white ? m_whiteKing : m_blackKing;
        if (king == s)
            return King;

        return None;
    }

    template PieceType BoardState::pieceOnSquare<true>(square s) const;
    template PieceType BoardState::pieceOnSquare<false>(square s) const;
}