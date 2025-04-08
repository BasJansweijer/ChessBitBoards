import math
import json

# We scale the elo slightly to acount for the fact that the engine is only given
# a limited amount of time to think which causes it to lose just by chance at times.
# The number was chosen by feeling, it is not based on any real data.
timeControllScalingFactor = 2.5

def elo_difference(wins, draws, losses):
    total_games = wins + draws + losses
    if total_games == 0:
        return 0

    actual_score = (wins + 0.5 * draws) / total_games

    # Avoid infinite values when score is 0 or 1
    epsilon = 1e-6
    actual_score = max(epsilon, min(1 - epsilon, actual_score))

    # Calculate Elo difference from perspective of player 1
    elo_diff = -400 * math.log10((1 / actual_score) - 1)
    return round(elo_diff, 2) * timeControllScalingFactor


def pathToVersion(path:str):
    # take only the version number
    name = path.split("-")[-1]
    return name

initialVersionElo = 1000


def getEloRatings():
    eloRatings = {}
    eloRatings["v0.1.0"] = initialVersionElo

    results = json.load(open("data/engineComparison.json"))

    # (e1, e2) -> eloDifference (positive if e2 is better)
    diffs = {}

    for result in results:
        engine1 = pathToVersion(result["engine1"])
        engine2 = pathToVersion(result["engine2"])
        e1Wins = result["engine1Wins"]
        draws = result["draws"]
        e2Wins = result["engine2Wins"]

        elo_diff = elo_difference(e2Wins, draws, e1Wins)
        diffs[(engine1, engine2)] = elo_diff
    
    # Calculate Elo ratings
    lastKnownVersion = "v0.1.0"
    
    while len(diffs) > 0:
        # Find the diffs entry where the first engine is the last known version
        for key in diffs.keys():
            if key[0] == lastKnownVersion:
                # Calculate the new Elo rating for the second engine
                eloRatings[key[1]] = eloRatings[key[0]] + diffs[key]
                lastKnownVersion = key[1]
                del diffs[key]
                break
        
    return eloRatings

if __name__ == "__main__":
    print(elo_difference(119, 435, 449))
    ratings = getEloRatings()
    for k, v in ratings.items():
        print(f"{k}: {v}")