#include <iostream>
#include <random>
#include "transposition.h"
#include <cinttypes>
#include <fstream>

struct Keys
{
    uint64_t pieceKeys[64][12];
    uint64_t castlingKeys[16];
    uint64_t enpassentKeys[16];
    uint64_t turnKey;
    // The last 20 plies we should hash differently
    uint64_t move50RuleKeys[20];
};

Keys initKeys(int seed)
{
    std::mt19937_64 rng(seed); // Fixed seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    Keys keys;

    for (int s = 0; s < 64; s++)
    {
        for (int pt = 0; pt < 12; pt++)
        {
            keys.pieceKeys[s][pt] = dist(rng);
        }
    }

    for (int ck = 0; ck < 16; ck++)
        keys.castlingKeys[ck] = dist(rng);

    for (int ep = 0; ep < 16; ep++)
        keys.enpassentKeys[ep] = dist(rng);

    keys.turnKey = dist(rng);

    for (int mr = 0; mr < 20; mr++)
        keys.move50RuleKeys[mr] = dist(rng);

    return keys;
}

void saveKeys(Keys k, std::string outFile)
{
    std::fstream out(outFile, std::ios::out);

    // turn key
    out << "constexpr key turnKey = " << k.turnKey << "ULL;";

    // castling keys
    out << "constexpr key castlingKeys[16] = {";
    bool first = true;
    for (int cr = 0; cr < 16; cr++)
    {
        if (!first)
            out << ", ";
        first = false;
        out << k.castlingKeys[cr] << "ULL";
    }
    out << "};" << std::endl;

    // enpassent key
    out << "constexpr key enpassentKeys[16] = {";
    first = true;
    for (int ep = 0; ep < 16; ep++)
    {
        if (!first)
            out << ", ";
        first = false;
        out << k.enpassentKeys[ep] << "ULL";
    }
    out << "};" << std::endl;

    // piece keys
    bool firstSquare = true;
    out << "constexpr key squarePieceKeys[64][12] = {";
    for (int s = 0; s < 64; s++)
    {
        if (!firstSquare)
            out << ", ";

        firstSquare = false;
        out << "{";

        bool firstKey = true;
        for (int pt = 0; pt < 12; pt++)
        {
            if (!firstKey)
                out << ", ";

            firstKey = false;

            out << k.pieceKeys[s][pt] << "ULL";
        }
        out << "}";
    }
    out << "};" << std::endl;

    // piece keys
    bool firstKey = true;
    out << "constexpr key move50RuleKeys[20] = {";
    for (int mk = 0; mk < 20; mk++)
    {
        if (!firstKey)
            out << ", ";

        firstKey = false;

        out << k.pieceKeys[mk] << "ULL";
    }
    out << "};" << std::endl;
}

int main()
{
    // This seed should be tweaked to get a "good" key set;
    constexpr int SEED = 1234;
    Keys k = initKeys(1234);
    saveKeys(k, "zobristKeys.out");
}
