import chess
from engineWrapper import ChessBoardGUI, ChessEngine


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

    result = None

    while not board.is_game_over():
        chessGui.update_board(board)

        resp = engine.bestMove(engineThinkSeconds)
        if resp == "Draw by 50 move rule":
            if not board.can_claim_fifty_moves():
                raise Exception(f"Incorrect 50 move rule on fen: {fen}")
            break

        move, info = resp
        print(f"Engine plays {move}, ({info})")

        engine.makeMove(move)
        board.push(chess.Move.from_uci(move))

        chessGui.update_board(board)

        if board.is_game_over():
            break

        # Let the user make a move
        move = getUserMove(board)
        engine.makeMove(move)
        board.push(chess.Move.from_uci(move))

    print("Game result:", board.result(claim_draw=True))
    input("enter to quit")

if __name__ == "__main__":
    engine = ChessEngine("./releases/engine-v0.7.2")
    playAgainstEngine(engine, 10, True, fen=STARTfen)
