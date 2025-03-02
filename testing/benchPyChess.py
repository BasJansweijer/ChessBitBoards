from chess import Board
from time import time

def runOnDepth(board: Board, depth):
    if depth == 0:
        return 1
    
    total = 1
  
    for move in board.legal_moves:
        newB = board.copy(stack=False)
        newB.push(move)
        total += runOnDepth(newB, depth - 1)
    
    return total


if __name__ == "__main__":
    b = Board()
    s = time()
    res = runOnDepth(b, 4)
    e = time()

    seconds = e-s
    print("Nodes searched", res)
    print("Time", seconds)
    print("Nodes / second", int(res/seconds))
    