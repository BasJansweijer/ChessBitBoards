import subprocess
import chess
import chess.svg
import cairosvg
import io
from tkinter import Tk, Canvas, PhotoImage
import re
from time import sleep


class ChessBoardGUI:
    def __init__(self):
        self.root = Tk()
        self.root.title("Chess Board")
        self.size = 600
        self.canvas = Canvas(self.root, width=self.size, height=self.size)
        self.canvas.pack()
        self.img = None  # We'll store the image here
        

        self.wait_on_init()
    
    def wait_on_init(self):
        while not self.root.winfo_exists():
            sleep(0.05)
        
        # self.root.mainloop()

    def update_board(self, board):

        # Generate SVG data for the board
        svg_data = chess.svg.board(board)
        
        # Convert the SVG to PNG in memory
        png_data = cairosvg.svg2png(bytestring=svg_data.encode('utf-8'), 
                                    output_width=self.size, output_height=self.size)
        
        # Create an in-memory image from the PNG data
        img_data = io.BytesIO(png_data)
        self.img = PhotoImage(data=img_data.read())
        
        # Update the canvas with the image
        self.canvas.create_image(0, 0, anchor="nw", image=self.img)
        self.root.update()

class ChessEngine:
    def __init__(self, engine_path):
        self.engine_path = engine_path
        self.process = subprocess.Popen([self.engine_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    def runCmd(self, cmd):
        self.process.stdin.write(cmd + '\n')
        self.process.stdin.flush()
        response = self.process.stdout.readline().strip()
        return response

    def getPosition(self):
        return self.runCmd("getPosition")

    def setPosition(self, fen):
        self.runCmd(f"setPosition {fen}")

    def bestMove(self, thinkTime):
        response = self.runCmd(f"bestMove {thinkTime}")
        pattern = r'(\S+) \(eval: (\S+), depth: (\d+)\)'
        match = re.search(pattern, response)
        
        if match:
            move = match.group(1)
            eval_score = match.group(2)
            depth = int(match.group(3))
            return move, eval_score, depth
        
        print(response)
        raise Exception("bestMove Not parsed correctly")

    def makeMove(self, uci_move):
        self.runCmd(f"makeMove {uci_move}")

    def quit(self):
        self.runCmd("quit")
        self.process.terminate()

def getUserMove(board: chess.Board) -> str:
    move:str = ''
    legalMoves: list[str] = [move.uci() for move in board.legal_moves]

    while (True):
        move = input("please provide a (uci) move: ")
        if move in legalMoves:
            return move
        
        print(f"'{move}' is not a legal (uci) move")


STARTfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
def playAgainstEngine(engine: ChessEngine, engineThinkSeconds, playWhite: bool, fen=STARTfen):
    chessGui = ChessBoardGUI()

    # Initialize the board using python-chess
    board = chess.Board(fen)
    engine.setPosition(fen)

    chessGui.update_board(board)

    if (playWhite):
        move = getUserMove(board)
        engine.makeMove(move)
        board.push(chess.Move.from_uci(move))

    while not board.is_game_over():
        chessGui.update_board(board)

        move, eval, maxDepth = engine.bestMove(engineThinkSeconds)
        print(f"Engine plays {move}, (est. eval: {eval}, searched depth: {maxDepth})")

        engine.makeMove(move)
        board.push(chess.Move.from_uci(move))

        chessGui.update_board(board)

        if board.is_game_over():
            break

        # Let the user make a move
        move = getUserMove(board)
        engine.makeMove(move)
        board.push(chess.Move.from_uci(move))

    print("Game result:", board.result())
    input("enter to quit")


if __name__ == "__main__":
    engine = ChessEngine("./build/app/engine")
    playAgainstEngine(engine, 10, True, fen=STARTfen)
