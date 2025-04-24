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
        std::regex goRegex("go wtime (\\d+) btime (\\d+)( winc (\\d+) binc (\\d+))?");
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
        else if (std::regex_match(cmd, match, goRegex))
        {
            Time wtime = std::stol(match[1]);
            Time btime = std::stol(match[2]);
            Time winc = 0;
            Time binc = 0;

            if (match[3].matched)
            {
                winc = std::stol(match[4]);
                binc = std::stol(match[5]);
            }

            ClockState clock(wtime, btime, winc, binc);
            int moveCounter = m_currentBoard.ply() / 2;
            Time thinkTime = m_currentBoard.whitesMove()
                                 ? clock.currentMoveTime<true>(moveCounter)
                                 : clock.currentMoveTime<false>(moveCounter);

            auto [move, eval, info] = findBestMove(thinkTime);
            double ttFullness = m_transTable.fullNess();
            std::cout << "info (eval: " << eval << ", searchinfo: " << info
                      << ", ttFullness: " << ttFullness
                      << ", spend time: " << (thinkTime / 1000) << ")" << std::endl;
            std::cout << "bestmove " << move.toUCI() << std::endl;
        }
        else if (std::regex_match(cmd, match, bestMoveRegex))
        {
            double seconds = std::stod(match[1]);
            if (m_currentBoard.drawBy50MoveRule())
            {
                std::cout << "Draw by 50 move rule" << std::endl;
                return;
            }

            auto [move, eval, info] = findBestMove(seconds * 1000);
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
                std::optional<BenchResult> result = bench<BenchType::Depth>(quantity);
                if (!result)
                {
                    std::cout << "Invalid benchmark configuration" << std::endl;
                    return;
                }

                BenchResult res = *result;
                std::cout << "Bench result: " << res.searchedNodes << " nodes in " << res.seconds
                          << " seconds (depth: " << res.depth << ")" << std::endl;
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