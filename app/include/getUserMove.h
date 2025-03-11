#pragma once

#include <iostream>

#include "chess.h"

chess::Move getUserMove(chess::BoardState b)
{
    chess::MoveList legalMoves = b.legalMoves();
    std::string userMove;
    while (true)
    {
        std::cout << "Enter a (uci) move: ";
        getline(std::cin, userMove);

        for (auto &m : legalMoves)
        {
            if (m.toUCI() == userMove)
                return m;
        }

        std::cout << "'" << userMove << "' is not a valid (uci) move." << std::endl;
    }
}