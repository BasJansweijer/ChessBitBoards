import subprocess
import chess
import chess.svg
import cairosvg
import io
from tkinter import Tk, Canvas, PhotoImage

class ChessBoardGUI:
    def __init__(self, root):
        self.root = root
        self.canvas = Canvas(root, width=400, height=400)
        self.canvas.pack()
        self.img = None  # We'll store the image here

    def update_board(self, board):
        # Generate SVG data for the board
        svg_data = chess.svg.board(board)
        
        # Convert the SVG to PNG in memory
        png_data = cairosvg.svg2png(bytestring=svg_data.encode('utf-8'))
        
        # Create an in-memory image from the PNG data
        img_data = io.BytesIO(png_data)
        self.img = PhotoImage(data=img_data.read())
        
        # Update the canvas with the image
        self.canvas.create_image(0, 0, anchor="nw", image=self.img)


root = Tk()
root.title("Chess Board")

# Create a ChessBoardGUI instance
chess_gui = ChessBoardGUI(root)

def run_gui():
    root.mainloop()

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
        move, _, evalStr = response.split()
        eval = int(evalStr[:-1])
        
        return move, eval

    def makeMove(self, uci_move):
        self.runCmd(f"makeMove {uci_move}")

    def quit(self):
        self.runCmd("quit")
        self.process.terminate()

def display_board(board):
    chess_gui.update_board(board)
    root.update()
 
def playGame(engine1Path, engine2Path, fen, thinkTime, verbose=False):
    engine1 = ChessEngine(engine1Path)
    engine2 = ChessEngine(engine2Path)

    # Initialize the board using python-chess
    board = chess.Board(fen)

    engine1.setPosition(fen)
    engine2.setPosition(fen)

    while not board.is_game_over():
        # Engine 1's move
        move1, eval = engine1.bestMove(thinkTime)
        if verbose:
            print(f"Engine 1 plays: {move1} (est. eval: {eval})")

        board.push(chess.Move.from_uci(move1))
        engine1.makeMove(move1)
        engine2.makeMove(move1)

        if verbose:
           display_board(board)

        # Check if game is over after Engine 1's move
        if board.is_game_over():
            break

        # Engine 2's move
        move2, eval = engine2.bestMove(thinkTime)
        if verbose:
            print(f"Engine 2 plays: {move2} (est. eval: {eval})")

        board.push(chess.Move.from_uci(move2))
        engine2.makeMove(move2)
        engine1.makeMove(move2)

        if verbose:
            display_board(board)

    # Game is over, show the result
    print(f"\nGame Over! Result: {board.result()}")

    # Quit engines
    engine1.quit()
    engine2.quit()


startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"
if __name__ == "__main__":
    playGame("../build/app/app", "../build/app/app", startFen, 0, verbose=True)