import chess
import chess.engine
import chess.pgn
import io
import random    

import sys
import os
import time

# Ensures python can find play.py
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from play import ChessBoardGUI, ChessEngine

def extract_fens(pgn_text, fenList):

    pgn = io.StringIO(pgn_text)
    game = chess.pgn.read_game(pgn)
    board = game.board()

    
    for move in game.mainline_moves():
        board.push(move)

        # add moves more regularly wen the ply is higher
        if random.random() < 0.0005 * board.ply():
            fenList.append(board.fen())

def getNfens(lichessDB, outfile, n):
    fens = []


    gamesFile = open(lichessDB)
    outfile = open(outfile, 'w')
    
    curGame = ""
    emptyLines = 0
    for line in gamesFile.readlines():
        curGame += line
        if line == "\n":
            emptyLines += 1
            if emptyLines %2 == 0:
                extract_fens(curGame, fens)
                curGame = ""
                if len(fens) >= n:
                    break

    for fen in fens[:n]:
        outfile.write(fen + '\n')
    
    outfile.close()
    gamesFile.close()

chessGui = ChessBoardGUI()

def extractOpeningFen(gamePNG: str, seenOpenings: dict[str, int]) -> None | str:
    lines = gamePNG.split('\n')

    openingName = ""
    for line in lines:
        if line.startswith("[Opening"):
            openingName = line.split("\"")[1]
    
    if openingName not in seenOpenings:
        seenOpenings[openingName] = 0

    openingCount = seenOpenings[openingName]

    pgn = io.StringIO(gamePNG)
    game = chess.pgn.read_game(pgn)
    if game == None:
        return
    
    board = game.board()

    movesIntoOpening = 10
    if openingCount > 1:
        if random.random() > 0.25:
            return
        movesIntoOpening += 10

    moves = 0
    for move in game.mainline_moves():
        moves += 1
        board.push(move)
        if moves == movesIntoOpening:
            break
    
    # chessGui.update_board(board)
    # time.sleep(0.5)
    
    seenOpenings[openingName] += 1

    return board.fen()

def roughlyEqual(fen: str, engine_path: str = "stockfish") -> bool:
    print(fen)
    board = chess.Board(fen)
    
    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        analysis = engine.analyse(board, chess.engine.Limit(time=1))
        score = analysis["score"].relative
        
        if score.is_mate():
           return False
        else:
            centipawn_score = score.score()
            print(f"Evaluation: {centipawn_score} centipawns")
            
            if -50 <= centipawn_score <= 50:
                return True
        
        return False

def getNequalOpeningPositions(lichessDB, outfile, n):
    seenOpenings = {}
    fens = set()

    gamesFile = open(lichessDB)
    outfile = open(outfile, 'w')
    
    curGame = ""
    emptyLines = 0
    for line in gamesFile.readlines():
        curGame += line
        if line == "\n":
            emptyLines += 1
            if emptyLines %2 == 0:
                fen = extractOpeningFen(curGame, seenOpenings)
                curGame = ""

                if fen is None or fen in fens:
                    continue

                if not roughlyEqual(fen):
                    continue

                chessGui.update_board(chess.Board(fen))
                fens.add(fen)
                if len(fens) >= n:
                    break

    for fen in fens:
        outfile.write(fen + '\n')
    
    outfile.close()
    gamesFile.close()


if __name__ == "__main__":
    lichessDBpath = "data/lichess_db_standard_rated_2013-01.pgn"

    # getNfens(lichessDBpath, "data/fens10000.txt", 10000)
    # getNfens(lichessDBpath, "data/fens10.txt", 10)
    # getNequalOpeningPositions(lichessDBpath, "data/openingFens.txt", 500)

