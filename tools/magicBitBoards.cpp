#include <iostream>
#include "bitBoard.h"
#include "boardVisualizer.h"
#include "moveConstants.h"
#include <vector>
#include <functional>
#include <random>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <future>
#include <regex>
#include "toolUtils.h"
#include "types.h"
#include "masks.h"

using bitboard = chess::bitboard;
using square = chess::square;

struct SquareMagic
{
    bitboard blockerMask;
    uint64_t magic;
    int arrSize;
};

void saveMagic(int s, bool rook, SquareMagic magic)
{
    static std::mutex fileMutex;
    std::lock_guard<std::mutex> lock(fileMutex);

    std::string outFile = rook ? "rookMagics.out" : "bishopMagics.out";
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

    out << "SquareMagic square" << s << " = {" << magic.blockerMask << ", " << magic.magic << ", " << magic.arrSize << "};\n";
    out.close();
}

// generates the mask for the blockers
bitboard blockerMaskBishop(square s)
{
    int rank = s / 8;
    int file = s % 8;

    bitboard moves = chess::constants::bishopMoves[s];

    return moves & ~chess::mask::edgeMask;
}

// generates the mask for the blockers
bitboard blockerMaskRook(square s)
{
    int rank = s / 8;
    int file = s % 8;

    bitboard moves = chess::constants::rookMoves[s];

    moves &= ~(1ULL << rank * 8);
    moves &= ~(1ULL << rank * 8 + 7);
    moves &= ~(1ULL << file);
    moves &= ~(1ULL << (8 * 7) + file);

    return moves;
}

bitboard blockedBishopMoves(square s, bitboard blockers)
{
    bitboard legalMoves = 0;
    int bishRank = s / 8;
    int bishFile = s % 8;

    int rank = bishRank;
    int file = bishFile;

    // up left
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank++;
        file--;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // up right
    rank = bishRank;
    file = bishFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank++;
        file++;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // down left
    rank = bishRank;
    file = bishFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank--;
        file--;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    // down right
    rank = bishRank;
    file = bishFile;
    while ((blockers & 1ULL << rank * 8 + file) == 0)
    {
        rank--;
        file++;
        if (!chess::bitBoards::inBounds(rank, file))
        {
            break;
        }
        legalMoves |= 1ULL << (rank * 8 + file);
    }

    return legalMoves;
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

std::vector<bitboard> blockerArangements(bitboard blockable)
{
    // retrieve a vector with all squares that we can block
    std::vector<square> blockableSquares;
    while (blockable)
    {
        square newBlocker = chess::bitBoards::firstSetBit(blockable);
        blockableSquares.push_back(newBlocker);
        blockable ^= 1ULL << newBlocker;
    }

    std::vector<bitboard> blockerArangements;

    int blockedSquares = 0; // each bit corresponds to an index in the blockableSquares vector (1 if blocked 0 if not).
    while (blockedSquares < 1 << (blockableSquares.size() + 1))
    {
        bitboard blockers = 0;
        for (int i = 0; i < blockableSquares.size(); i++)
        {
            if (blockedSquares & 1 << i)
            {
                blockers |= 1ULL << blockableSquares[i];
            }
        }

        blockerArangements.push_back(blockers);

        // move to next blocker arangement
        blockedSquares++;
    }

    return blockerArangements;
}

static std::random_device rd;                                       // Obtain a random seed
static std::mt19937_64 gen(rd());                                   // 64-bit random number generator
static std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX); // Distribution between 0 and UINT64_MAX
thread_local std::vector<uint64_t> foundMagics;

static int maxTriesPerArrSize = 100000;

bool tryWithArrSize(int arraySize, const std::vector<std::pair<bitboard, bitboard>> &blockerToMoves, SquareMagic &outMagic)
{
    std::vector<bitboard> resultingMovesArr(arraySize, 0);

    int totalSuccesfullInsertions = 0;

    // use the already found magic as a first attempt
    int foundM = 0;
    uint64_t magic;
    if (foundMagics.size() != 0)
        magic = foundMagics[0];

    int tries = 0;
    while (tries < maxTriesPerArrSize)
    {
        tries++;

        bool mappingSuccess = true;

        for (auto [blockers, legalMoves] : blockerToMoves)
        {
            int idx = blockers * magic % arraySize;
            if (resultingMovesArr[idx] != 0 && resultingMovesArr[idx] != legalMoves)
            {
                mappingSuccess = false;
                break;
            }

            resultingMovesArr[idx] = legalMoves;
            totalSuccesfullInsertions++;
        }

        int avgUntillCollision = totalSuccesfullInsertions / (tries);
        if (avgUntillCollision < 400 && tries > 10)
        {
            return false;
        }

        if (mappingSuccess)
        {
            outMagic.magic = magic;
            outMagic.arrSize = arraySize;
            return true;
        }

        // Clear out the resultingMovesArr
        std::fill(resultingMovesArr.begin(), resultingMovesArr.end(), 0);

        foundM++;
        if (foundM < foundMagics.size())
            magic = foundMagics[foundM];
        else
            // choose a new magic
            magic = dist(gen);
    }

    return false;
}
static int maxConsequtiveFails = 100000;
static bool verbose = false;
SquareMagic generateBlockedMagic(square s,
                                 std::function<bitboard(square)> getBlockerMask,
                                 std::function<bitboard(square, bitboard)> getNonBLockedMoves, bool increasedSize = false)
{
    SquareMagic generatedMagic;
    generatedMagic.arrSize = -1;
    // some initial magic i found.
    generatedMagic.blockerMask = getBlockerMask(s);

    std::vector<bitboard> allBlockers = blockerArangements(generatedMagic.blockerMask);
    std::vector<std::pair<bitboard, bitboard>> blockerToMoves;
    for (bitboard blockers : allBlockers)
    {
        blockerToMoves.emplace_back(blockers, getNonBLockedMoves(s, blockers));
    }

    int arraySize = pow(2, __builtin_popcountll(generatedMagic.blockerMask)) + 1;
    if (increasedSize)
        arraySize *= 4 + 1;

    int smallestFoundArraySize = arraySize * 2 + 1;
    int consequtiveFails = 0;
    while (consequtiveFails < maxConsequtiveFails)
    {
        bool found = tryWithArrSize(arraySize, blockerToMoves, generatedMagic);
        if (found)
        {
            if (verbose)
                std::cout << "best size: " << arraySize << "(magic: " << generatedMagic.magic << ')' << std::endl;

            smallestFoundArraySize = arraySize;
            if (std::find(foundMagics.begin(), foundMagics.end(), generatedMagic.magic) == foundMagics.end())
                foundMagics.push_back(generatedMagic.magic);
            consequtiveFails = 0;
        }
        else
        {
            consequtiveFails++;
            // If we are too low we can search closer
            // to the smallest array found thus far.
            if (arraySize < smallestFoundArraySize * 0.75 && consequtiveFails > 500)
            {
                arraySize = smallestFoundArraySize;
            }
        }

        // Lower our array size to attempt at a different size (skip even sizes)
        if (arraySize < 5)
        {
            arraySize = smallestFoundArraySize;
        }
        arraySize -= 2;

        if (verbose && consequtiveFails % (maxConsequtiveFails / 100) == 0 && consequtiveFails != 0)
            std::cout << consequtiveFails / (double)maxConsequtiveFails << '%' << std::endl;
    }

    return generatedMagic;
}

static int maxRetries = 10;

void generateAndSaveRookMagic(square s, int retries = 0)
{
    foundMagics.clear();
    bool increaseSize = retries > maxRetries / 2;
    int square = static_cast<int>(s);
    SquareMagic foundMagic = generateBlockedMagic(s, blockerMaskRook, blockedRookMoves, increaseSize);
    if (foundMagic.arrSize == -1)
    {
        if (retries < maxRetries)
            generateAndSaveRookMagic(s, retries + 1);
        else
            std::cout << "Unable to find magic for square " << square << std::endl;
    }
    else
    {
        std::cout << "saving square " << square
                  << " (final size: " << foundMagic.arrSize << ")" << std::endl;
        saveMagic(s, true, foundMagic);
    }
}

void generateRookMagics()
{
    std::vector<std::future<void>> futures;
    for (square s = 0; s < 64; s++)
    {
        futures.push_back(std::async(std::launch::async, generateAndSaveRookMagic, s, 0));
    }

    for (auto &f : futures)
    {
        f.get();
    }
}

void generateAndSaveBishopMagic(square s, int retries = 0)
{
    foundMagics.clear();
    bool increaseSize = retries > maxRetries / 2;
    int square = static_cast<int>(s);
    SquareMagic foundMagic = generateBlockedMagic(s, blockerMaskBishop, blockedBishopMoves, increaseSize);
    if (foundMagic.arrSize == -1)
    {
        if (retries < maxRetries)
            generateAndSaveBishopMagic(s, retries + 1);
        else
            std::cout << "Unable to find magic for square " << square << std::endl;
    }
    else
    {
        std::cout << "saving square " << square
                  << " (final size: " << foundMagic.arrSize << ")" << std::endl;
        saveMagic(s, false, foundMagic);
    }
}

void generateBishopMagics()
{
    std::vector<std::future<void>> futures;
    for (square s = 0; s < 64; s++)
    {
        futures.push_back(std::async(std::launch::async, generateAndSaveBishopMagic, s, 0));
    }

    for (auto &f : futures)
    {
        f.get();
    }
}

void testBlockersToMoves(const chess::constants::MagicInfo &m, square s, const std::vector<bitboard> &moveArr)
{
    std::vector<bitboard> allBlockers = blockerArangements(m.mask);
    std::vector<std::pair<bitboard, bitboard>> blockerToMoves;
    for (bitboard blockers : allBlockers)
    {
        blockerToMoves.emplace_back(blockers, blockedRookMoves(s, blockers));
    }

    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister PRNG

    // Define the range for distribution
    std::uniform_int_distribution<size_t> dist(0, blockerToMoves.size() - 1);
    auto [blockers, moves] = blockerToMoves[dist(gen)];

    uint32_t idx = m.arrayOffset + ((m.magic * blockers) % m.squareArraySize);

    std::cout << "Off: " << m.arrayOffset << " size: " << m.squareArraySize << " final idx: " << idx << std::endl;
    chess::bitBoards::showBitboardGUI(blockers, "Blockers");
    chess::bitBoards::showBitboardGUI(moveArr[idx], "Moves");
}

void writeMagicArray(chess::constants::MagicInfo (&magics)[64], const std::vector<bitboard> moves, bool rook)
{
    std::string piece = rook ? "rook" : "bishop";

    int endIdx = moves.size() - 1;
    while (moves[endIdx] == 0)
        endIdx--;

    int finalSize = endIdx + 1;

    // Get the current working directory
    std::filesystem::path currentDir = std::filesystem::current_path();
    // Combine the current directory with outFile name
    std::filesystem::path fullPath = currentDir / "magicMoveArrays.out";

    std::fstream out(fullPath, std::ios::app);
    if (!out)
    {
        std::cerr << "Failed to open file: " << fullPath << std::endl;
        return;
    }

    out << "constexpr MagicInfo " << piece << "Magics" << "[64] = {";
    bool first = true;
    for (auto m : magics)
    {
        if (!first)
            out << ", ";

        first = false;

        out << "{" << m.mask << "ULL, " << m.magic << "ULL, "
            << m.squareArraySize << "U, " << m.arrayOffset << "U}";
    }
    out << "};\n";

    out << "constexpr bitboard " << piece << "NonBlockedMoves" << "[" << finalSize << "] = {";
    first = true;
    for (int i = 0; i < finalSize; i++)
    {
        if (!first)
            out << ", ";

        first = false;

        out << moves[i] << "ULL";
    }
    out << "};\n";
    out.close();

    std::cout << "Saved at: " << fullPath << std::endl;
}

void readMagicsAndCreateArray(bool rooks, bool showTest = false)
{
    chess::constants::MagicInfo magics[64];

    std::string outFile = rooks ? "rookMagics.out" : "bishopMagics.out";
    // Get the current working directory
    std::filesystem::path currentDir = std::filesystem::current_path();
    // Combine the current directory with outFile name
    std::filesystem::path fullPath = currentDir / outFile;

    std::ifstream out(fullPath);
    std::string line;

    int totalArrSize = 0;

    std::regex magicFormat("SquareMagic square(\\d+) = \\{(\\d+), (\\d+), (\\d+)\\};");
    std::smatch regexMatch;
    while (getline(out, line))
    {
        if (std::regex_search(line, regexMatch, magicFormat))
        {
            int square = std::stoi(regexMatch[1].str());
            bitboard mask = std::stoull(regexMatch[2].str());
            bitboard magic = std::stoull(regexMatch[3].str());
            int arraySize = std::stoi(regexMatch[4].str());
            totalArrSize += arraySize;

            magics[square].mask = mask;
            magics[square].magic = magic;
            magics[square].squareArraySize = arraySize;
        }
    }

    std::cout << "Theoretical array size: " << totalArrSize << std::endl;
    std::vector<bitboard> legalMoveArray(totalArrSize, 0);
    int curOffset = 0;

    for (square s = 0; s < 64; s++)
    {
        chess::constants::MagicInfo &m = magics[s];
        m.arrayOffset = curOffset;

        std::vector<bitboard> allBlockers = blockerArangements(m.mask);
        std::vector<std::pair<bitboard, bitboard>> blockerToMoves;
        for (bitboard blockers : allBlockers)
        {
            if (rooks)
                blockerToMoves.emplace_back(blockers, blockedRookMoves(s, blockers));
            else
                blockerToMoves.emplace_back(blockers, blockedBishopMoves(s, blockers));
        }

        for (auto [blockers, moves] : blockerToMoves)
        {
            uint32_t idx = m.arrayOffset + (blockers * m.magic) % m.squareArraySize;
            legalMoveArray[idx] = moves;
        }

        curOffset += m.squareArraySize;
        // start the new array on the first location without a board in it.
        while (legalMoveArray[curOffset] == 0)
            curOffset--;
        curOffset++;
    }

    int endIdx = legalMoveArray.size() - 1;
    while (legalMoveArray[endIdx] == 0)
        endIdx--;

    std::cout << "Final array size: " << endIdx + 1 << std::endl;

    writeMagicArray(magics, legalMoveArray, rooks);

    if (showTest)
    {
        for (square s = 0; s < 64; s++)
            testBlockersToMoves(magics[s], s, legalMoveArray);
    }
}

int main()
{
    // generateAndSaveRookMagic(0, 4);
    // generateAndSaveRookMagic(7, 4);
    // generateRookMagics();
    // readMagicsAndCreateArray(true, true);

    // the bishop array is smaller so we can get away with doing more attemps
    // maxConsequtiveFails = 1000000;
    // maxTriesPerArrSize = 1000000;
    // generateBishopMagics();

    clearFile("magicMoveArrays.out");
    readMagicsAndCreateArray(true, false);
    readMagicsAndCreateArray(false, false);
}