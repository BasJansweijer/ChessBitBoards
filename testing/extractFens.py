import chess
import chess.pgn
import io
import random    


def extract_fens(pgn_text, fenList):

    pgn = io.StringIO(pgn_text)
    game = chess.pgn.read_game(pgn)
    board = game.board()

    
    for move in game.mainline_moves():
        board.push(move)

        # add moves more regularly wen the ply is higher
        if random.random() < 0.0005 * board.ply():
            fenList.append(board.fen())
    
if __name__ == "__main__":
    lichessDB = open("data/lichess_db_standard_rated_2013-01.pgn")
    outputFile = open("data/fens.txt", 'w')
    
    fens = []
    
    curGame = ""
    emptyLines = 0
    for line in lichessDB.readlines():
        curGame += line
        if line == "\n":
            emptyLines += 1
            if emptyLines %2 == 0:
                extract_fens(curGame, fens)
                curGame = ""
                if len(fens) >= 10000:
                    break

    for fen in fens:
        outputFile.write(fen + '\n')

