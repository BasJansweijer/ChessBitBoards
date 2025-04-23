import subprocess
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

        # Ensure old image is deleted before assigning a new one
        if self.img:
            del self.img  # Free the old image memory
        
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
    
    def benchDepth(self, depth):
        response = self.runCmd(f"bench depth {depth}")
        pattern = r'\n took (d+(.d+)?) seconds'
        match = re.search(pattern, response)
        
        if match:
            time = match.group(1)
            nodes = match.group(2)
            return time, nodes
        
        print("failed on response:", response)
        raise Exception("benchDepth Not parsed correctly")

    def getPosition(self):
        return self.runCmd("getPosition")

    def setPosition(self, fen):
        self.runCmd(f"setPosition {fen}")

    def bestMove(self, thinkTime):
        response = self.runCmd(f"bestMove {thinkTime}")
        pattern = r'(\S+) \(([^)]+)\)'
        match = re.search(pattern, response)
        
        if match:
            move = match.group(1)
            info = match.group(2)
            return move, info
        
        if response.startswith("Draw by 50 move rule"):
            return "Draw by 50 move rule"
        
        print("failed on response:", response)
        raise Exception("bestMove Not parsed correctly")

    def makeMove(self, uci_move):
        self.runCmd(f"makeMove {uci_move}")

    def quit(self):
        self.runCmd("quit")
        self.process.terminate()