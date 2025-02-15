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
        m_whitePawns = 0xFF00;
        m_whiteKnights = 0b01000010;
        m_whiteBishops = 0b00100100;
        m_whiteRooks = 0b10000001;
        m_whiteKing = 0b00010000;
        m_whiteQueens = 0b00001000;

        m_blackPawns = m_whitePawns << 8 * 5;
        m_blackKnights = m_whiteKnights << 8 * 7;
        m_blackBishops = m_whiteBishops << 8 * 7;
        m_blackRooks = m_whiteRooks << 8 * 7;
        m_blackQueens = m_whiteQueens << 8 * 7;
        m_blackKing = m_whiteKing << 8 * 7;

        m_enpassentLocations = 0;
        m_whiteCanCastleLong = true;
        m_whiteCanCastleShort = true;
        m_blackCanCastleLong = true;
        m_blackCanCastleShort = true;
        m_whitesMove = true;
    }

    BoardState::BoardState(std::string_view fen) : m_whitePawns(0), m_whiteKnights(0), // overwrite the existing memory to default
                                                   m_whiteBishops(0), m_whiteRooks(0),
                                                   m_whiteQueens(0), m_whiteKing(0),
                                                   m_blackPawns(0), m_blackKnights(0),
                                                   m_blackBishops(0), m_blackRooks(0),
                                                   m_blackQueens(0), m_blackKing(0),
                                                   m_enpassentLocations(0), m_whiteCanCastleLong(false),
                                                   m_whiteCanCastleShort(false), m_blackCanCastleLong(false),
                                                   m_blackCanCastleShort(false), m_whitesMove(true)
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
                chess::bitBoards::setBit(m_blackPawns, rank, file);
                break;
            case 'n':
                chess::bitBoards::setBit(m_blackKnights, rank, file);
                break;
            case 'b':
                chess::bitBoards::setBit(m_blackBishops, rank, file);
                break;
            case 'r':
                chess::bitBoards::setBit(m_blackRooks, rank, file);
                break;
            case 'q':
                chess::bitBoards::setBit(m_blackQueens, rank, file);
                break;
            case 'k':
                chess::bitBoards::setBit(m_blackKing, rank, file);
                break;
            case 'P':
                chess::bitBoards::setBit(m_whitePawns, rank, file);
                break;
            case 'N':
                chess::bitBoards::setBit(m_whiteKnights, rank, file);
                break;
            case 'B':
                chess::bitBoards::setBit(m_whiteBishops, rank, file);
                break;
            case 'R':
                chess::bitBoards::setBit(m_whiteRooks, rank, file);
                break;
            case 'Q':
                chess::bitBoards::setBit(m_whiteQueens, rank, file);
                break;
            case 'K':
                chess::bitBoards::setBit(m_whiteKing, rank, file);
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
            chess::bitBoards::setBit(m_enpassentLocations, rank, file);
        }
    }
}