#pragma once

#include <vector>
#include <string>
#include <boost/process.hpp>
#include <memory>
#include <map>
#include <chrono>

namespace bp = boost::process;

namespace stockfish
{

    using perftResult = std::unordered_map<std::string, uint64_t>;
    class Engine
    {
    public:
        Engine()
        {
        }

        void startEngine()
        {
            if (m_started)
                return;

            m_engineProc = std::make_unique<bp::child>("./stockfish", bp::std_in<m_outStream, bp::std_out> m_inStream);
            m_started = true;
        }

        ~Engine()
        {
            sendCommand("quit");
        }

        void setPosition(std::string fen)
        {
            sendCommand("position fen " + fen);
        }

        // Returns a map which for each legal moves gives the leaf nodes at the depth.
        perftResult perft(int depth);

        // Returns the nps (nodes per second)
        int benchMoveGen(int depth, std::string fenFile);

    private:
        bool m_started = false;
        std::unique_ptr<bp::child> m_engineProc;
        bp::ipstream m_inStream;
        bp::opstream m_outStream;

        void sendCommand(const std::string &command)
        {
            // ensure the engine is running
            startEngine();
            m_outStream << command << std::endl;
        }
    };

}