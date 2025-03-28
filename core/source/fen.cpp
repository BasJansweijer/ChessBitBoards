
#include "bitBoard.h"
#include "chess.h"
#include <iostream>

namespace chess
{
    BoardState::BoardState(std::string_view fen)
        : m_enpassentSquare(-1), m_whitesMove(true),
          m_castleRights(0)
    {
        for (int i = 0; i < 5; i++)
        {
            m_blackPieces[i] = 0;
            m_whitePieces[i] = 0;
        }

        int rank = 7;
        int file = 0;
        for (char c : fen)
        {
            if (c == ' ')
                break;

            if (c == '/')
            {
                // new rank
                rank--;
                file = 0;
                continue;
            }

            switch (c)
            {
            case 'p':
                chess::bitBoards::setBit(m_blackPieces[PieceType::Pawn], rank, file);
                break;
            case 'n':
                chess::bitBoards::setBit(m_blackPieces[PieceType::Knight], rank, file);
                break;
            case 'b':
                chess::bitBoards::setBit(m_blackPieces[PieceType::Bishop], rank, file);
                break;
            case 'r':
                chess::bitBoards::setBit(m_blackPieces[PieceType::Rook], rank, file);
                break;
            case 'q':
                chess::bitBoards::setBit(m_blackPieces[PieceType::Queen], rank, file);
                break;
            case 'k':
                m_blackKing = rank * 8 + file;
                break;
            case 'P':
                chess::bitBoards::setBit(m_whitePieces[PieceType::Pawn], rank, file);
                break;
            case 'N':
                chess::bitBoards::setBit(m_whitePieces[PieceType::Knight], rank, file);
                break;
            case 'B':
                chess::bitBoards::setBit(m_whitePieces[PieceType::Bishop], rank, file);
                break;
            case 'R':
                chess::bitBoards::setBit(m_whitePieces[PieceType::Rook], rank, file);
                break;
            case 'Q':
                chess::bitBoards::setBit(m_whitePieces[PieceType::Queen], rank, file);
                break;
            case 'K':
                m_whiteKing = rank * 8 + file;
                break;
            default:
                break;
            }
            if (std::isdigit(c))
            {
                file += c - '0';
            }
            else
            {
                file++;
            }
        }

        // parsing whose move it is
        int spacePos = fen.find(' ');
        if (spacePos == std::string::npos)
            throw std::runtime_error("fen format incorrect");

        char color = fen.at(spacePos + 1);
        m_whitesMove = color == 'w';

        // parsing who can castle
        int curIdx = spacePos + 3;
        while (fen.at(curIdx) != ' ')
        {
            switch (fen.at(curIdx))
            {
            case 'K':
                m_castleRights |= 0b1;
                break;
            case 'Q':
                m_castleRights |= 0b10;
                break;
            case 'k':
                m_castleRights |= 0b100;
                break;
            case 'q':
                m_castleRights |= 0b1000;
                break;
            }
            curIdx++;
        }

        // parse enpassant square
        curIdx++;
        if (fen.at(curIdx) != '-')
        {
            int file = fen.at(curIdx) - 'a';
            int rank = fen.at(curIdx + 1) - '1';
            m_enpassentSquare = rank * 8 + file;
        }

        // Ensure an up to date hash
        recomputeHash();
    }

    char BoardState::charOnSquare(square s) const
    {
        bitboard location = 1ULL << s;

        if (m_whitePieces[PieceType::Pawn] & location)
            return 'P';
        else if (m_blackPieces[PieceType::Pawn] & location)
            return 'p';
        else if (m_whitePieces[PieceType::Knight] & location)
            return 'N';
        else if (m_blackPieces[PieceType::Knight] & location)
            return 'n';
        else if (m_whitePieces[PieceType::Bishop] & location)
            return 'B';
        else if (m_blackPieces[PieceType::Bishop] & location)
            return 'b';
        else if (m_whitePieces[PieceType::Rook] & location)
            return 'R';
        else if (m_blackPieces[PieceType::Rook] & location)
            return 'r';
        else if (m_whitePieces[PieceType::Queen] & location)
            return 'Q';
        else if (m_blackPieces[PieceType::Queen] & location)
            return 'q';
        else if (getWhiteKing() & location)
            return 'K';
        else if (getBlackKing() & location)
            return 'k';

        return 0;
    }

    std::string BoardState::fen() const
    {
        std::string fen = "";
        int noPieceCount = 0;
        bool first = true;
        for (int rank = 7; rank >= 0; rank--)
        {
            if (!first)
                fen += '/';

            first = false;

            for (int file = 0; file < 8; file++)
            {
                char p = charOnSquare(rank * 8 + file);
                if (p == 0)
                {
                    noPieceCount++;
                    continue;
                }

                if (noPieceCount > 0)
                {
                    fen += (char)noPieceCount + '0';
                    noPieceCount = 0;
                }

                fen += p;
            }

            if (noPieceCount > 0)
            {
                fen += (char)noPieceCount + '0';
                noPieceCount = 0;
            }
        }

        fen += m_whitesMove ? " w " : " b ";

        std::string castleRights = "";
        if (whiteCanCastleShort())
            castleRights += "K";
        if (whiteCanCastleLong())
            castleRights += "Q";
        if (blackCanCastleShort())
            castleRights += "k";
        if (blackCanCastleLong())
            castleRights += "q";

        if (castleRights == "")
            castleRights = "-";

        fen += castleRights;

        if (m_enpassentSquare != UINT8_MAX)
        {
            char rank = m_enpassentSquare / 8 + '1';
            char file = m_enpassentSquare % 8 + 'a';
            fen += ' ';
            fen += file;
            fen += rank;
            fen += ' ';
        }
        else
            fen += " - ";

        return fen;
    }

}