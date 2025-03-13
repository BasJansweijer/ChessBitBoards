import subprocess
import chess
import chess.svg
import cairosvg
import io
from tkinter import Tk, Canvas, PhotoImage
import re
from play import ChessBoardGUI, ChessEngine

 
def playGame(engine1Path, engine2Path, fen, thinkTime, verbose=False):
    if verbose:
        chessGui = ChessBoardGUI()

    engine1 = ChessEngine(engine1Path)
    engine2 = ChessEngine(engine2Path)

    # Initialize the board using python-chess
    board = chess.Board(fen)

    engine1.setPosition(fen)
    engine2.setPosition(fen)

    while not board.is_game_over():
        # Engine 1's move
        move1, eval, maxDepth = engine1.bestMove(thinkTime)
        if verbose:
            print(f"Engine 1 plays: {move1} (est. eval: {eval}, maxDepth: {maxDepth})")

        board.push(chess.Move.from_uci(move1))
        engine1.makeMove(move1)
        engine2.makeMove(move1)

        if verbose:
           chessGui.update_board(board)

        # Check if game is over after Engine 1's move
        if board.is_game_over():
            break

        # Engine 2's move
        move2, eval, maxDepth = engine2.bestMove(thinkTime)
        if verbose:
            print(f"Engine 2 plays: {move2} (est. eval: {eval}, maxDepth: {maxDepth})")

        board.push(chess.Move.from_uci(move2))
        engine2.makeMove(move2)
        engine1.makeMove(move2)

        if verbose:
            chessGui.update_board(board)

    # Game is over, show the result
    print(f"\nGame Over! Result: {board.result()}")

    # Quit engines
    engine1.quit()
    engine2.quit()


startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"
if __name__ == "__main__":
    playGame("../build/app/app", "../build/app/app", startFen, 0.25, verbose=True)