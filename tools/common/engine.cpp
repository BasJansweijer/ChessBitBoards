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
}