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

        bool gameFinished()
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

        // Returns the best found move, evaluation (int), max completed depth (int)
        std::tuple<Move, int, int> findBestMove(double thinkSeconds)
        {
            Search s(currentBoard, evaluate);
            return s.iterativeDeepening(thinkSeconds);
        }

        BoardState board() const { return currentBoard; }
        void setPosition(BoardState b) { currentBoard = b; }

    private:
        bool quit;
        BoardState currentBoard;
    };

}
