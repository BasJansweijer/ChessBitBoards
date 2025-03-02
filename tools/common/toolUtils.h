#pragma once
#include "bitBoard.h"
#include <vector>
#include <string>
#include <chrono>

void writeArray(std::vector<bitboard> array, const std::string &outFile, std::string_view arrName);

void clearFile(const std::string &outFile);

namespace utils
{
    class Timer
    {
    public:
        Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

        ~Timer()
        {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;
            accumulated_time += elapsed.count();
        }

        static double getAccumulatedTime() { return accumulated_time; }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        static inline double accumulated_time = 0.0;
    };
}
