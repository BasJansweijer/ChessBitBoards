#include "engine.h"
#include <iostream>
#include <regex>

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
        std::smatch match;

        if (std::regex_match(cmd, match, setPosRegex))
        {
            std::string fen = match[1];
            currentBoard = BoardState(fen);

            std::cout << "done" << std::endl;
        }
        else if (cmd.starts_with("getPosition"))
        {
            std::cout << currentBoard.fen() << std::endl;
        }
        else if (std::regex_match(cmd, match, makeMoveRegex))
        {
            makeMove(match[1]);
            std::cout << "done" << std::endl;
        }
        else if (std::regex_match(cmd, match, bestMoveRegex))
        {
            double seconds = std::stod(match[1]);
            auto [move, eval, depth] = findBestMove(seconds);
            std::cout << move.toUCI() << " (eval: " << eval << ", depth: " << depth << ")" << std::endl;
        }
        else if (cmd == "quit" || cmd == "exit")
        {
            quit = true;
            std::cout << "done" << std::endl;
        }
        else
            cmdInvallid(cmd);
    }
}