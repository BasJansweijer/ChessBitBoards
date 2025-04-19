#include <iostream>
#include <fstream>
#include "engine.h"

#define GREEN "\033[32m"

void updateProgress(int progress)
{
    int numChars = 4;
    for (int i = 0; i < numChars; i++)
        std::cout << '\b';

    std::string text = std::to_string(progress);
    text += '%';
    while (text.size() < numChars)
        text += ' ';
    std::cout << text;
    std::cout.flush();
}

int numTestFens(std::string fensPath)
{
    std::ifstream fensFile(fensPath);

    return std::count(std::istreambuf_iterator<char>(fensFile),
                      std::istreambuf_iterator<char>(), '\n');
}

void benchTestFens(int testDepth, std::string fensPath)
{

    int numFens = numTestFens(fensPath);

    std::ifstream fensFile(fensPath);
    std::cout << "Starting movegeneration test on " << numFens << " positions " << std::endl;
    int progress = 0.0;
    int fensTested = 0;

    double totalTime = 0.0;
    uint64_t totalNodes = 0;

    std::string fen;
    while (getline(fensFile, fen))
    {
        chess::BoardState b(fen);
        chess::Engine e;
        e.setPosition(b);

        std::optional<chess::Engine::BenchResult> result = e.bench<chess::Engine::BenchType::Depth>(testDepth);
        if (!result)
        {
            std::cout << "failed on fen:\n"
                      << fen << std::endl;
            break;
        }

        totalNodes += (*result).searchedNodes;
        totalTime += (*result).seconds;

        fensTested++;
        progress = 100 * fensTested / numFens;
        updateProgress(progress);
    }

    double avgNodes = static_cast<double>(totalNodes) / numFens;
    double avgTime = totalTime / numFens;
    std::cout << "\nAverage nodes searched: " << avgNodes
              << "\nAverage time: " << avgTime << std::endl;
}

int main(int argc, char *argv[])
{
    // The quick mode is usefull for faster itteration when experimenting with optimizations
    bool quickMode = false;

    // Loop through command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--quick")
        {
            quickMode = true;
        }
    }

    std::string fensFile = quickMode ? "testing/fens10.txt" : "testing/fens10000.txt";
    benchTestFens(5, fensFile);
}
