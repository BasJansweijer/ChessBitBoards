#include <iostream>
#include <string>
#include "chess.h"
#include "boardVisualizer.h"
#include "engine.h"

int main()
{
    chess::Engine engine;
    while (!engine.hasQuit())
    {
        std::string cmd;
        getline(std::cin, cmd);
        engine.runCmd(cmd);
    }
}
