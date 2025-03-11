#include <iostream>
#include "chess.h"
#include "boardVisualizer.h"
#include "engine.h"
#include "getUserMove.h"

int main()
{
    chess::BoardState b;
    while (true)
    {
        chess::showBoardGUI(b);
        chess::Move userMove = getUserMove(b);
        b.makeMove(userMove);
        chess::showBoardGUI(b);
        std::cout << "Start thinking" << std::endl;
        chess::Move engineMove = chess::engine::findBestMove(b, 6);
        b.makeMove(engineMove);
    }
}
