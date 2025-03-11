#pragma once

#include "chess.h"
#include "search.h"
#include "eval.h"
#include <iostream>
#include <string>
#include <tuple>

namespace chess
{

    class Engine
    {
    public:
        Engine() = default;
        Engine(BoardState startPosition) : currentBoard(startPosition), quit(false) {}

        // The main way users will interface with the engine
        void runCmd(std::string cmd);

        bool hasQuit() { return quit; }

        bool isCheckmate()
        {
            return currentBoard.legalMoves().size() == 0;
        }

        bool moveIsLegal(std::string uciMove)
        {
            for (auto &m : currentBoard.legalMoves())
                if (m.toUCI() == uciMove)
                    return true;

            return false;
        }

        // Returns false if move is illegal
        bool makeMove(std::string uciMove)
        {
            for (auto &m : currentBoard.legalMoves())
            {
                if (m.toUCI() == uciMove)
                {
                    currentBoard.makeMove(m);
                    return true;
                }
            }

            return false;
        }

        // Returns the best found move and an int (the evaluation)
        std::pair<Move, int> findBestMove(int depth)
        {
            Move best;
            int eval;
            if (currentBoard.whitesMove())
                eval = minimax<true, true>(currentBoard, evaluate, depth, best);
            else
                eval = minimax<false, true>(currentBoard, evaluate, depth, best);

            return {best, eval};
        }

        BoardState board() const { return currentBoard; }
        void setPosition(BoardState b) { currentBoard = b; }

    private:
        bool quit;
        BoardState currentBoard;
    };

}
