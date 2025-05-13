import chess
import chess.svg
import os
import sys
import json
import argparse


# Ensures python can find play.py
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from play import ChessBoardGUI, ChessEngine

chessGui: ChessBoardGUI | None = None

def playGame(engine1: ChessEngine, engine2: ChessEngine, fen, thinkTime, verbose=False):
    global chessGui

    if verbose and chessGui is None:
        chessGui = ChessBoardGUI()

    # Initialize the board using python-chess
    board = chess.Board(fen)

    engine1.setPosition(fen)
    engine2.setPosition(fen)


    if verbose:
       chessGui.update_board(board)

    while not board.is_game_over():
        print(board.fen())
        # Engine 1's move
        resp = engine1.bestMove(thinkTime)
        if resp == "Draw by 50 move rule":
            if not board.can_claim_fifty_moves():
                raise Exception(f"Incorrect 50 move rule on fen: {fen}")
            print(resp)
            break

        move1, info = resp
        if verbose:
            print(f"{engine1.engine_path} plays: {move1} ({info})")

        board.push(chess.Move.from_uci(move1))
        engine1.makeMove(move1)
        engine2.makeMove(move1)

        if verbose:
           chessGui.update_board(board)

        # Check if game is over after Engine 1's move
        if board.is_game_over():
            break

        # Engine 2's move
        resp = engine2.bestMove(thinkTime)
        if resp == "Draw by 50 move rule":
            if not board.can_claim_fifty_moves():
                raise Exception(f"Incorrect 50 move rule on fen: {fen}")
            print(resp)
            break

        move2, info = resp

        if verbose:
            print(f"{engine2.engine_path} plays: {move2} ({info})")

        board.push(chess.Move.from_uci(move2))
        engine2.makeMove(move2)
        engine1.makeMove(move2)

        if verbose:
            chessGui.update_board(board)

    # Game is over, show the result
    print(f"\nGame Over! Result: {board.result(claim_draw=True)}")
    return board.result(claim_draw=True)

    

def compareEngines(engine1: ChessEngine, engine2: ChessEngine, thinkTime=0.5, fensFile="data/openingFens.txt", verbose=True):
    draws = 0
    engine1Wins = 0
    engine2Wins = 0

    n = 0
    for fen in open(fensFile):
        n += 1

        print(f"Current results: engine1 wins: {engine1Wins}, engine2 wins: {engine2Wins}, draws: {draws}")
        print(f"Starting on fen {n} ({fen})")

        fen = fen.rstrip('\n')
        print(f"white: {engine1.engine_path}, black: {engine2.engine_path}")
        res1 = playGame(engine1, engine2, fen, thinkTime, verbose=verbose)
        if res1 == "1-0":
            engine1Wins += 1
        elif res1 == "0-1":
            engine2Wins += 1
        elif res1 == "1/2-1/2":
            draws += 1


        print(f"white: {engine2.engine_path}, black: {engine1.engine_path}")
        res2 = playGame(engine2, engine1, fen, thinkTime, verbose=verbose)
        if res2 == "1-0":
            engine2Wins += 1
        elif res2 == "0-1":
            engine1Wins += 1
        elif res2 == "1/2-1/2":
            draws += 1
    
    return (engine1Wins, draws, engine2Wins)

compResultFile = "data/engineComparison.json"

def storeComparison(engine1: ChessEngine, engine2: ChessEngine, result:tuple[int, int, int]):
    global compResultFile

    # Check if file exists, if not, create it and initialize with an empty list
    if not os.path.exists(compResultFile):
        with open(compResultFile, 'w') as f:
            f.write("[]")  # Write an empty list to the file

    # Prepare the result data
    res = {
        "engine1": engine1.engine_path, 
        "engine2": engine2.engine_path,
        "engine1Wins": result[0], 
        "draws": result[1], 
        "engine2Wins": result[2]
    }
    
    # Read the existing data
    with open(compResultFile, 'r') as f:
        results = json.load(f)

    # Append the new comparison result
    results.append(res)
    
    # Write the updated results back to the file
    with open(compResultFile, 'w') as f:
        json.dump(results, f, indent=4)  # Pretty-print with indentation for readability
        


startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"
if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="use --quick to run 100 fens instead of 10000")
    parser.add_argument('--quick', action='store_true', help='Run quick benchmark with 100 fens')
    args = parser.parse_args()

    if args.quick:
        fensFile = "data/openingFens100.txt"
    else:
        fensFile = "data/openingFens.txt"

    print(f"Using fens file {fensFile}")

    e1 = ChessEngine("../releases/engine-v0.7.6")
    e2 = ChessEngine("../releases/engine-v0.7.7")

    result = compareEngines(e1, e2, fensFile=fensFile)
    storeComparison(e1, e2, result)

    # Quit engines
    e1.quit()
    e2.quit()