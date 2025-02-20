/*
This file contains the implementation of both initializing the start position and initializing
from a given fen position.
*/

#include "bitBoard.h"
#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>

namespace chess
{

    BoardState::BoardState()
    {
        // Setup board:
        m_white.pawns = 0xFF00;
        m_white.knights = 0b01000010;
        m_white.bishops = 0b00100100;
        m_white.rooks = 0b10000001;
        m_white.king = 0b00010000;
        m_white.queens = 0b00001000;

        m_black.pawns = m_white.pawns << 8 * 5;
        m_black.knights = m_white.knights << 8 * 7;
        m_black.bishops = m_white.bishops << 8 * 7;
        m_black.rooks = m_white.rooks << 8 * 7;
        m_black.queens = m_white.queens << 8 * 7;
        m_black.king = m_white.king << 8 * 7;

        m_enpassentSquare = -1;
        m_whiteCanCastleLong = true;
        m_whiteCanCastleShort = true;
        m_blackCanCastleLong = true;
        m_blackCanCastleShort = true;
        m_whitesMove = true;
    }

    BoardState::BoardState(std::string_view fen)
        : m_white({0, 0, 0, 0, 0, 0}), m_black({0, 0, 0, 0, 0, 0}),
          m_enpassentSquare(-1), m_whitesMove(true),
          m_whiteCanCastleLong(false), m_whiteCanCastleShort(false),
          m_blackCanCastleLong(false), m_blackCanCastleShort(false)
    {
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
                chess::bitBoards::setBit(m_black.pawns, rank, file);
                break;
            case 'n':
                chess::bitBoards::setBit(m_black.knights, rank, file);
                break;
            case 'b':
                chess::bitBoards::setBit(m_black.bishops, rank, file);
                break;
            case 'r':
                chess::bitBoards::setBit(m_black.rooks, rank, file);
                break;
            case 'q':
                chess::bitBoards::setBit(m_black.queens, rank, file);
                break;
            case 'k':
                chess::bitBoards::setBit(m_black.king, rank, file);
                break;
            case 'P':
                chess::bitBoards::setBit(m_white.pawns, rank, file);
                break;
            case 'N':
                chess::bitBoards::setBit(m_white.knights, rank, file);
                break;
            case 'B':
                chess::bitBoards::setBit(m_white.bishops, rank, file);
                break;
            case 'R':
                chess::bitBoards::setBit(m_white.rooks, rank, file);
                break;
            case 'Q':
                chess::bitBoards::setBit(m_white.queens, rank, file);
                break;
            case 'K':
                chess::bitBoards::setBit(m_white.king, rank, file);
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
                m_whiteCanCastleShort = true;
                break;
            case 'Q':
                m_whiteCanCastleLong = true;
                break;
            case 'k':
                m_blackCanCastleShort = true;
                break;
            case 'q':
                m_blackCanCastleLong = true;
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
    }
}