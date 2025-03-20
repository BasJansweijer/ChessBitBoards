import os
import sys

# Ensures python can find play.py
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from play import ChessBoardGUI, ChessEngine
from enginePlayout import playGame

fens = [
    "8/8/8/3k4/8/8/8/6QK w - - 0 1",
    "8/8/8/3k4/8/8/8/6RK w - - 0 1",
    "8/8/8/3k4/8/8/8/5BNK w - - 0 1"
]

if __name__ == "__main__":
    e1 = ChessEngine("../build/app/engine")
    e2 = ChessEngine("../build/app/engine")
    for fen in fens:
        playGame(e1, e2, fen, 0.25, True)