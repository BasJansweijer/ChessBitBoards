#include "toolUtils.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "types.h"

using bitboard = chess::bitboard;

void clearFile(const std::string &outFile)
{
    std::fstream out(std::filesystem::current_path() / outFile, std::ios::out);
    out.close();
}

void writeArray(std::vector<bitboard> array, const std::string &outFile, std::string_view arrName)
{
    // Get the current working directory
    std::filesystem::path currentDir = std::filesystem::current_path();
    // Combine the current directory with outFile name
    std::filesystem::path fullPath = currentDir / outFile;

    std::fstream out(fullPath, std::ios::app);
    if (!out)
    {
        std::cerr << "Failed to open file: " << fullPath << std::endl;
        return;
    }

    out << "constexpr bitboard " << arrName << "[" << array.size() << "] = {";
    bool first = true;
    for (auto i : array)
    {
        if (!first)
            out << ", ";
        out << i << "ULL";
        first = false;
    }
    out << "};\n";
    out.close();

    std::cout << "Saved at: " << fullPath << std::endl;
}