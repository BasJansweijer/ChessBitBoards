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


if __name__ == "__main__":
    lichessDBpath = "data/lichess_db_standard_rated_2013-01.pgn"

    getNfens(lichessDBpath, "data/fens10000.txt", 10000)
    getNfens(lichessDBpath, "data/fens10.txt", 10)

