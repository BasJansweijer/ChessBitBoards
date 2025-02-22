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
        startEngine();
        sendCommand("go perft " + std::to_string(depth));

        std::regex moveFormat("(\\w\\d\\w\\d): (\\d+)");
        std::smatch regexMatch;

        bool movesStarted = false;
        bool foundMoves = false;

        perftResult moves;

        std::string line;
        while (getline(m_inStream, line))
        {
            if (std::regex_search(line, regexMatch, moveFormat))
            {
                movesStarted = true;
                foundMoves = true;
                moves.emplace(regexMatch[1], std::stoull(regexMatch[2]));
            }
            else
                foundMoves = false;

            if (movesStarted && !foundMoves)
                break;
        }

        return moves;
    }
}