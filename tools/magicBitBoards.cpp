#include <iostream>
#include "bitBoard.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>

struct blockedMoves
{
    std::vector<bitboard> legalMoves;
    uint8_t magic;

    // legal moves is found by using the bitboard with any blockers set to 1
    // legalMoves[(blockers * magic) >> (64 - idxBitSize)]
};

// generates the mask for the blockers
bitboard blockerMaskRook(square s, bitboard moves)
{
    int rank = s / 8;
    int file = s % 8;

    moves &= ~(1ULL << rank * 8);
    moves &= ~(1ULL << rank * 8 + 7);
    moves &= ~(1ULL << file);
    moves &= ~(1ULL << (8 * 7) + file);

    return moves;
}

bitboard blockedRookMoves(square s, bitboard blockers)
{
    bitboard legalMoves = 0;
    int rookRank = s / 8;
    int rookFile = s % 8;
    int rank = rookRank;
    int file = rookFile;

    // scan up
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank++;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // scan down
    rank = rookRank;
    file = rookFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank--;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // scan left
    rank = rookRank;
    file = rookFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        file--;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // scan right
    rank = rookRank;
    file = rookFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        file++;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    return legalMoves;
}

std::unordered_map<bitboard, bitboard> genNonBlockedMoves(square s, bitboard blockerMask, std::function<bitboard(square, bitboard)> nonBlockedPieceMoves)
{
    std::vector<square> blockableSquares;

    std::unordered_map<bitboard, bitboard> blockerToMoves;

    while (blockerMask)
    {
        square movable = chess::bitBoards::firstSetBit(blockerMask);
        // remove this move
        blockerMask ^= 1ULL << movable;
        blockableSquares.push_back(movable);
    }

    // Each bit represents an index in the movableSquares vector
    // 1 means contains a blocker and 0 means no blocker;
    int16_t presentBlockers = 0;

    // while we have less bits used than there are possible blocker locations
    while (presentBlockers < (1 << blockableSquares.size()))
    {
        bitboard blockers = 0;
        for (int i = 0; i < blockableSquares.size(); i++)
        {
            // if should be present put it in the bitboard
            if (presentBlockers & (1 << i))
                blockers |= 1ULL << blockableSquares[i];
        }

        bitboard legalMoves = nonBlockedPieceMoves(s, blockers);
        blockerToMoves[blockers] = legalMoves;

        // go to next combination
        presentBlockers++;
    }

    return blockerToMoves;
}

int bitSpreadHeuristic(uint64_t magic)
{
    return __builtin_popcountll(magic);
}

void optimizeBlockersToMoveMap(std::unordered_map<bitboard, bitboard> &unOptimizedMap, blockedMoves &outRes)
{
    // Random number generator engine
    std::random_device rd;                                       // Obtain a random seed
    std::mt19937_64 gen(rd());                                   // 64-bit random number generator
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX); // Distribution between 0 and UINT64_MAX

    // start with 64 unique indices (2^6 = 64)
    int arrSize = 2001;

    int triesPerSize = 1000000;

    while (true)
    {
        // We skip even magics since these always have the least significant bit 0;'
        int tries = 0;
        int nonColisions = 0;
        while (tries < triesPerSize)
        {
            // if collisions always occure soon in the insertion process this array size
            // is likely not nice
            int avgUntillCollision = nonColisions / (tries + 1);
            if (avgUntillCollision < 375 && tries > 50)
            {
                break;
            }

            tries++;
            uint64_t magic = dist(gen);

            // int numSetBits = bitSpreadHeuristic(magic);
            // if (numSetBits < 32 || numSetBits > 40)
            //     continue;

            // std::cout << magic << std::endl;

            // create a new empty mappings
            std::vector<bitboard> legalMoveMap(arrSize, 0);
            bool sucessfullMapping = true;

            for (const auto &[blockers, moves] : unOptimizedMap)
            {
                int index = (blockers * magic) % arrSize;
                // std::cout << index << std::endl;

                if (legalMoveMap[index] != 0 && legalMoveMap[index] != moves)
                {
                    // std::cout << "col after " << nonCol << std::endl;
                    sucessfullMapping = false;
                    break;
                }

                legalMoveMap[index] = moves;
                nonColisions++;
            }

            if (sucessfullMapping)
            {
                std::cout << "SUCCESS: " << "tries: " << tries << "colAvg " << (nonColisions / tries) << " size: " << arrSize << " magic: " << magic << std::endl;
                outRes.legalMoves = legalMoveMap;
                outRes.magic;
                break;
            }
        }

        arrSize += 2;

        if (arrSize + 1000 < outRes.legalMoves.size())
        {
            std::cout << "Couldn't find anything for last 50 array sizes" << std::endl;
        }
    }
}

std::vector<blockedMoves> genBlockedMoves(std::vector<bitboard> possibleMoves)
{
    std::vector<blockedMoves> result(64);
    int curIdx = 0;
    for (square s = 0; s < possibleMoves.size(); s++)
    {
        bitboard moves = possibleMoves[s];
        bitboard blockerMask = blockerMaskRook(s, moves);
        auto blockersToMoves = genNonBlockedMoves(s, blockerMask, blockedRookMoves);
        optimizeBlockersToMoveMap(blockersToMoves, result[s]);
    }
    return result;
}

int main()
{
}