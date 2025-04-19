#include "engine.h"
#include <iostream>
#include <regex>

#include "boardVisualizer.h"

void cmdInvallid(std::string cmd)
{
    std::cout << "'" << cmd << "' is not a valid command" << std::endl;
}

namespace chess
{

    void Engine::runCmd(std::string cmd)
    {
        std::regex setPosRegex("setPosition ([\\w/0-9\\-\\s]+)");
        std::regex bestMoveRegex("bestMove (\\d+(\\.\\d+)?)");
        std::regex makeMoveRegex("makeMove (\\w+)");
        std::regex benchmarkRegex("bench (\\w+) (\\d+(\\.\\d+)?)");
        std::smatch match;

        if (std::regex_match(cmd, match, setPosRegex))
        {
            std::string fen = match[1];
            setPosition(BoardState(fen));

            std::cout << "done" << std::endl;
        }
        else if (cmd.starts_with("getPosition"))
            std::cout << m_currentBoard.fen() << std::endl;
        else if (std::regex_match(cmd, match, makeMoveRegex))
        {
            bool succes = makeMove(match[1]);
            if (succes)
                std::cout << "done" << std::endl;
            else
                std::cout << "'" << match[1] << "' is not a legal move!" << std::endl;
        }
        else if (std::regex_match(cmd, match, bestMoveRegex))
        {
            double seconds = std::stod(match[1]);
            if (m_currentBoard.drawBy50MoveRule())
            {
                std::cout << "Draw by 50 move rule" << std::endl;
                return;
            }

            auto [move, eval, info] = findBestMove(seconds);
            double ttFullness = m_transTable.fullNess();
            std::cout << move.toUCI() << " (eval: " << eval << ", searchinfo: " << info
                      << ", ttFullness: " << ttFullness << ")" << std::endl;
        }
        else if (std::regex_match(cmd, match, benchmarkRegex))
        {
            std::string benchType = match[1];
            double quantity = std::stod(match[2]);
            if (benchType == "depth")
            {
                bench<BenchType::Depth>(quantity);
                return;
            }

            std::cout << "Invalid benchmark type: " << benchType << std::endl;
        }
        else if (cmd == "showBoard" || cmd == "show")
        {
            std::cout << m_currentBoard.fen() << std::endl;
            showBoardGUI(m_currentBoard);
        }
        else if (cmd == "quit" || cmd == "exit")
        {
            m_quit = true;
            std::cout << "done" << std::endl;
        }
        else
            cmdInvallid(cmd);
    }
}