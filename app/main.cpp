#include "chess.h"
#include "boardVisualizer.h"
#include <iostream>

int main()
{
    chess::BoardState b;
    showBoardGUI(b, b.getBlackBishops());
}
