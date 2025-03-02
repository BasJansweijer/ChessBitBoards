#include "stockfish.h"
#include <iostream>
#include <vector>
#include <string>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <regex>

namespace stockfish
{
    perftResult Engine::perft(int depth)
    {
        sendCommand("go perft " + std::to_string(depth));

        std::regex moveFormat("(\\w\\d\\w\\d\\w?): (\\d+)");
        std::smatch regexMatch;

        perftResult moves;

        std::string line;
        while (getline(m_inStream, line))
        {
            if (std::regex_search(line, regexMatch, moveFormat))
                moves.emplace(regexMatch[1], std::stoull(regexMatch[2]));

            if (line.starts_with("Nodes searched:"))
                break;
        }

        return moves;
    }

    int Engine::benchMoveGen(int depth, std::string fensFile)
    {
        sendCommand("bench 16 1 " + std::to_string(depth) + ' ' + fensFile + " perft");
        std::string line;

        std::regex nodesPerSecFormat("Nodes/second    : (\\d+)");
        std::smatch regexMatch;

        int res = -1;

        while (getline(m_inStream, line))
        {
            std::cout << "LINE" << line << std::endl;
            if (std::regex_search(line, regexMatch, nodesPerSecFormat))
            {
                std::cout << "MATCH" << std::endl;
                res = std::stoi(regexMatch[1]);
                break;
            }
        }

        return res;
    }
}