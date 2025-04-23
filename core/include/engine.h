#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <optional>
#include "chess.h"
#include "search.h"
#include "eval.h"
#include "repetitionTable.h"
#include "transposition.h"
#include "timeman.h"

namespace chess
{

    class Engine
    {
    public:
        struct EngineConfig
        {
            // Default
            EngineConfig()
                : transpositionTableMBs(64) {};
            // Default 64 mb table
            int transpositionTableMBs;
        };

        // Always need a "default config"
        Engine(EngineConfig config = EngineConfig())
            : m_quit(false), m_transTable(config.transpositionTableMBs)
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

            // Handle 50 move rule
            if (chosenMove.resets50MoveRule())
                m_repTable.clear();

            m_currentBoard.makeMove(chosenMove);

            // Add the current board to the reperition table
            bool success = m_repTable.addState(m_currentBoard);

            return true;
        }

        // Returns the best found move, evaluation (int), max completed depth (int)
        std::tuple<Move, Eval, Search::SearchStats> findBestMove(Time thinkTime)
        {
            Search::SearchConfig config;
            config.evalFunction = Evaluator::evaluate;
            config.repTable = &m_repTable;
            config.transTable = &m_transTable;

            Search s(m_currentBoard, config);
            return s.iterativeDeepening(thinkTime);
        }

        BoardState board() const { return m_currentBoard; }
        void setPosition(BoardState b)
        {
            m_currentBoard = b;

            m_transTable.clear();
            m_repTable.clear();

            m_repTable.addState(m_currentBoard);
        }

        enum class BenchType
        {
            Depth
        };

        struct BenchResult
        {
            double seconds;
            int searchedNodes;
            int depth;
        };

        template <BenchType benchType>
        std::optional<BenchResult> bench(double quantity);

    private:
        bool m_quit;
        BoardState m_currentBoard;

        // If we have 100 previous states then the 50 move rule aplies
        // Note that we store a version of the hash which does not contain any enpassant key.
        RepetitionTable m_repTable;
        TranspositionTable m_transTable;
    };

}
