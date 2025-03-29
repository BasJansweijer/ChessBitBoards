#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include "chess.h"
#include "search.h"
#include "eval.h"
#include "repetitionTable.h"

namespace chess
{
    class Engine
    {
    public:
        Engine() = default;
        Engine(BoardState startPosition) : m_currentBoard(startPosition), m_quit(false)
        {
            m_repTable.addState(m_currentBoard);
        }

        // The main way users will interface with the engine
        void runCmd(std::string cmd);

        bool hasQuit() { return m_quit; }

        bool gameFinished()
        {
            return m_currentBoard.legalMoves().size() == 0;
        }

        bool moveIsLegal(std::string uciMove)
        {
            for (auto &m : m_currentBoard.legalMoves())
                if (m.toUCI() == uciMove)
                    return true;

            return false;
        }

        // Returns false if move is illegal
        bool makeMove(std::string uciMove)
        {
            Move chosenMove = Move::Null();
            // Find the chosen move
            for (auto &m : m_currentBoard.legalMoves())
            {
                if (m.toUCI() == uciMove)
                {
                    chosenMove = m;
                    break;
                }
            }

            // Check if we found a move.
            if (chosenMove.isNull())
                return false;

            m_currentBoard.makeMove(chosenMove);

            /*
             * Handle the repetition table
             */

            if (chosenMove.resets50MoveRule())
                m_repTable.clear();

            if (m_repTable.drawDueTo50MoveRule())
            {
                // TODO: handle 50 move rule reached
                return true;
            }

            // Add the current board to the reperitio
            m_repTable.addState(m_currentBoard);

            return true;
        }

        // Returns the best found move, evaluation (int), max completed depth (int)
        std::tuple<Move, Eval, Search::SearchStats> findBestMove(double thinkSeconds)
        {
            Search::SearchConfig config;
            config.evalFunction = evaluate;
            config.repTable = &m_repTable;

            Search s(m_currentBoard, config);
            return s.iterativeDeepening(thinkSeconds);
        }

        BoardState board() const { return m_currentBoard; }
        void setPosition(BoardState b) { m_currentBoard = b; }

    private:
        bool m_quit;
        BoardState m_currentBoard;

        // If we have 100 previous states then the 50 move rule aplies
        // Note that we store a version of the hash which does not contain any enpassant key.
        RepetitionTable m_repTable;
    };

}
