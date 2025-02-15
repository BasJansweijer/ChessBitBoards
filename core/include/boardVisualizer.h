#pragma once
#include "chess.h"

namespace chess
{
    namespace bitBoards
    {
        void showBitboardGUI(bitboard bb, const std::string &windowName = "BitBoard");
    }

    void showBoardGUI(const BoardState &board, bitboard highlights = 0, const std::string &windowName = "ChessBoard");
}