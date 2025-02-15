#include <iostream>
#include "boardVisualizer.h"
#include "toolUtils.h"
#include <vector>
#include <functional>

std::vector<bitboard> genMovesArray(std::function<bitboard(int, int)> genMovesForPos)
{
    std::vector<bitboard> legalMoves(64);
    for (int i = 0; i < 64; i++)
    {
        int rank = i / 8;
        int file = i % 8;
        bitboard moves = genMovesForPos(rank, file);
        legalMoves[i] = moves;
    }
    return legalMoves;
}

bitboard knightMoves(int rank, int file)
{
    constexpr std::pair<int, int> moveDirs[4] = {
        {2, 1},
        {2, -1},
        {1, 2},
        {1, -2}};

    int moveRank;
    int moveFile;
    bitboard moves = 0;
    for (auto &m : moveDirs)
    {
        moveRank = rank + m.first;
        moveFile = file + m.second;
        if (chess::bitBoards::inBounds(moveRank, moveFile))
        {
            chess::bitBoards::setBit(moves, moveRank, moveFile);
        }

        moveRank = rank - m.first;
        if (chess::bitBoards::inBounds(moveRank, moveFile))
        {
            chess::bitBoards::setBit(moves, moveRank, moveFile);
        }
    }
    moves;

    return moves;
}

bitboard rookMoves(int rank, int file)
{
    return chess::bitBoards::rankMask(rank) ^ chess::bitBoards::fileMask(file);
}

bitboard bishopMoves(int rank, int file)
{
    int curDiagUpFile = file - rank;
    int curDiagDownFile = file + rank;

    bitboard moves = 0;

    for (int r = 0; r < 8; r++)
    {
        if (chess::bitBoards::inBounds(r, curDiagDownFile))
            chess::bitBoards::setBit(moves, r, curDiagDownFile);

        if (chess::bitBoards::inBounds(r, curDiagUpFile))
            chess::bitBoards::setBit(moves, r, curDiagUpFile);

        curDiagUpFile += 1;
        curDiagDownFile -= 1;
    }

    chess::bitBoards::unSetBit(moves, rank, file);
    return moves;
}

void showMoveArray(std::vector<bitboard> legalMoves)
{
    for (auto bb : legalMoves)
    {
        chess::bitBoards::showBitboardGUI(bb);
    }
}

int main()
{
    const std::string outFile = "moveArrays.out";
    clearFile(outFile);
    std::vector<bitboard> knightMoveArr = genMovesArray(knightMoves);
    writeArray(knightMoveArr, outFile, "knightMoves");

    std::vector<bitboard> rookMoveArr = genMovesArray(rookMoves);
    writeArray(knightMoveArr, outFile, "rookMoves");

    std::vector<bitboard> bishopMoveArr = genMovesArray(bishopMoves);
    showMoveArray(bishopMoveArr);
    writeArray(bishopMoveArr, outFile, "bishopMoves");
}