#pragma once
#include "chess.h"

namespace chess
{
    namespace bitBoards
    {
        void showBitboardGUI(bitboard bb);
    }

    void showBoardGUI(const BoardState &board, bitboard highlights = 0);
}