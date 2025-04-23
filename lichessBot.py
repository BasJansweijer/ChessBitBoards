import berserk.exceptions
from engineWrapper import ChessEngine
import os
import re
from packaging import version 
import berserk
import threading
from dotenv import load_dotenv
from time import sleep

RELEASES_DIR = "releases/" 
def getLatestReleasePath():
    # Pattern for engine-v0.x.x where x is a number
    pattern = re.compile(r"engine-v(0\.\d+\.\d+)$")

    releases = []
    for filename in os.listdir(RELEASES_DIR):
        match = pattern.match(filename)
        if match:
            ver = match.group(1)
            releases.append(ver)

    if not releases:
        return None

    # Sort using version comparison
    latest_version = max(releases, key=version.parse)
    return f"./{RELEASES_DIR}engine-v{latest_version}"

latestEngineExecutable = getLatestReleasePath()

# Set the berserk session and client
load_dotenv()
lichessToken = os.environ.get("LICHESS_BOT_TOKEN")
if lichessToken is None:
    raise Exception("Missing required enviroment variable 'LICHESS_BOT_TOKEN'")

session = berserk.TokenSession(lichessToken)
client = berserk.Client(session=session)

def gameStartEvent(event):
    white_player = event['white']['id']
    black_player = event['black']['id']
    my_id = client.account.get()['id'].lower()
    opponent = black_player if my_id == white_player.lower() else white_player

    print(f"Started game against {opponent}")

def gameWelcome(game_id):
    msgs = ["Hi, I'm BOTrePly, on my plies I will reply with a (hopefully) good move.",
            "I'm an engine developed by Bas Jansweijer, for info on my inner bits and bobs see https://github.com/BasJansweijer/ChessBitBoards"]
    
    sleep(0.5)
    client.bots.post_message(game_id, msgs[0])
    sleep(1)
    client.bots.post_message(game_id, msgs[1])

def setEngineToCurrentPosition(engine: ChessEngine, moves):
    startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    engine.setPosition(startFen)
    for move in moves:
        engine.makeMove(move)

def playGame(game_id):
    # Fetch initial game info
    ongoingGame = None
    for game in client.games.get_ongoing():
        if game_id == game['gameId']:
            ongoingGame = game
            break

    if ongoingGame is None:
        return
    
    lastMove = ongoingGame['lastMove']
    engineIsWhite = ongoingGame['color'] == 'white'

    engine = ChessEngine(latestEngineExecutable)
    
    if lastMove != '':
        # Set to current position
        engine.setPosition(ongoingGame['fen'])
        if ongoingGame['isMyTurn']:
            move, _ = engine.bestMove(10)
            client.bots.make_move(game_id, move)
    else:
        threading.Thread(target=gameWelcome, args=(game_id,)).start()

    for event in client.bots.stream_game_state(game_id):
        match event['type']:
            case 'gameFull':
                gameStartEvent(event)
            case 'gameState':
                if event['status'] != 'started':
                    break
                
                # Catch up to current position
                moves = event['moves'].split(' ')
                lastMoveIdx = -1
                while lastMove != '' and moves[lastMoveIdx] != lastMove:
                    lastMoveIdx += 1

                # make missed moves on engine board
                for move in moves[lastMoveIdx+1:]:
                    engine.makeMove(move)
                lastMove = moves[-1]

                whiteToMove = len(moves) % 2 == 0
                if whiteToMove != engineIsWhite:
                    continue

                # If it is our turn we make a move
                engineMove, _ = engine.bestMove(10)
                client.bots.make_move(game_id, engineMove)
            case "chatLine":
                pass
            case "opponentGone":
                print("opponent left")
            case _:
                print(f"unknown event type in game {event['type']}")

    # game done
    engine.quit()

def isStandardGame(challenge):
    variant_ok = challenge["variant"]["name"] == "Standard"
    return variant_ok

def correctTimeControll(challenge):
    speed_ok = challenge["speed"] in ["bullet", "blitz", "rapid", "classical"]
    timeControl_ok = challenge["timeControl"]["type"] == "clock"
    return speed_ok and timeControl_ok

def handleEvents():
    for event in client.bots.stream_incoming_events():
        match event['type']:
            case 'challenge':
                challenge_id = event['challenge']['id']
                if (not isStandardGame(event['challenge'])):
                    client.bots.decline_challenge(challenge_id, reason="standard")
                    continue

                if (not correctTimeControll(event['challenge'])):
                    client.bots.decline_challenge(challenge_id, reason="timeControl")
                    continue

                client.bots.accept_challenge(challenge_id)
            case 'gameStart':
                game_id = event['game']['id']
                threading.Thread(target=playGame, args=(game_id,)).start()
            case 'gameFinish':
                print(event)
            case _:
                print(f"Unkown event type '{event['type']}'")

if __name__ == "__main__":
    handleEvents()