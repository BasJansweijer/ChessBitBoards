#include <iostream>
#include <string>
#include "chess.h"
#include "boardVisualizer.h"
#include "engine.h"

int main(int argc, char *argv[])
{
    chess::Engine::EngineConfig config;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if ((arg == "-ttMbs" || arg == "--ttMbs") && i + 1 < argc)
        {

            config.transpositionTableMBs = std::stoi(argv[i + 1]);
            i++;
        }
    }

    chess::Engine engine(config);
    while (!engine.hasQuit())
    {
        std::string cmd;
        getline(std::cin, cmd);
        engine.runCmd(cmd);
    }
}
