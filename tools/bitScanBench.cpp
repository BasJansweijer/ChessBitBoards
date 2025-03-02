#include "bitBoard.h"
#include "toolUtils.h"
#include <random>
#include <iostream>
#include <bitset>
#include <chrono>


int myBitScan(bitboard bb)
{
    bitboard mask = 1;
    int i = 0;
    while (!(bb & (mask << i)) && i < 64)
        i++;
    return i;
}

int main()
{
    // Define a seed (e.g., 12345)
    uint64_t seed = 12345;

    // Initialize a 64-bit Mersenne Twister with the seed
    std::mt19937_64 rng(seed);

    // Define a uniform distribution over the full uint64_t range
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    std::vector<bitboard> nums(1000000);

    for (int i = 0; i < nums.size(); i++)
    {
        uint64_t random_number = dist(rng);
        nums[i] = random_number;
    }

    {
        Timer t;
        for (auto i : nums)
        {
            bitScanForward(i);
        }
    }

    {
        Timer t;
        for (auto i : nums)
        {
            myBitScan(i);
        }
    }

    return 0;
}
