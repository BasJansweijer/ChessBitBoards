# Engine improvements

To track how much the engine is improving throughout development the `testing/enginePlayout.py` script is used to let each new version play 1000 games against the previous version.

## Initial version (v0.1.0)

The initial version was very basic, it used iterative deepening an minimax to make its moves and used only a basic evaluation containing the a score for the material on the board, some additional scores for the positions of each square and a king safety term.

## Improved endgames (v0.2.0)

This version improved the mop up score from version v0.0.1 by testing and tweaking it such that the basic mates at the end of the game became consist.
Additionally seperate score tables for each piece in the endgame were added. We then interpolated using a [0,1] endgameNess score between using the middle game tables from v0.0.1 and the new endgame tables. From watching some of the games it definitly seemed to consistently win endgames which were not completely lost.
After some testing we also tweaked it a bit so the mopup score is less strong when the opponent has more material as well as pawn pushes being encouraged a bit more.
The final results of this against the previous version were: 278 wins, 488 draws and 234 losses. (~30 elo increase)

## Quiescent search (v0.3.0)

The evaluation function that is used does not take into account that, on the next move, there might be pieces that can be captured. To prevent 'halucinations' in the minimax outcome we implement quiescent search to always explore al the capture possibilities at the end of the search.
Doing so improved the performance by roughly 230 elo. The results against the previous version were 449 wins, 435 draws and 116 losses.

## Repitition detection (v0.4.0)

We add zobrist hashing and utilize a transposition table to detect when previous positions are repeated. Even though the nps of the move generation decrease by a bit under 20% and the search code also now needs to check for repetitions the depth we can search seems to have slightly increased due to the branches we can stop searching due to them being repetitions.
This addition resulted in 389 wins, 192 losses and 419 draws against the previous version (~125 elo increase).
Interestingly the number of draws did not decrease much, this is probably because in losing positions this version can try to get a draw on purpose while the previous version doesn't prevent the draw.

## Transposition table (v0.5.0)

Using the zobrist keys we add a transposition table. This table is persistant between searches and only purges old entries on collisions. The transposition table is filled with TTEntry struct and can store an exact, upper or lower bound for a position. In some cases this allows us to skip the search of a position if a good enough entry is stored for that position. The table is filled an used in the full search. Additionally the quiescent search uses entries aswell but doesn't store results.
This addition resulted in 465 wins, 162 draws and 373 wins against the previous version (~40 elo increase). Though we suspect that the strength increase is larger in slower time controls as with only 0.5 seconds for each move the depth of the entries in the tables is never very high.

## Evaluation Improvements (v0.6.x)

This improvement mostly consists of small tweaks and additions to the evaluation function.
To evaluate each individual improvement we only ran on 100 positions instead of on 1000.

### Rework queen square table (v0.6.1)

Before quiescent search was added the queen was always too eager to come out early. To discourage this the square table was tweaked. We now revert this change as we don't necessarily want to discourage the queen from comming out. Achieved 273 wins, 59 draws and 192 losses.

### Encourage trading down when ahead (v0.6.2)

To encourage trading pieces we add 0.2 times the percentage of non pawn material times the material balance to the evaluation. This encourages us to trade non pawn pieces when the material balance is in our favor. (Initialy we tried to use the endgameness score but this is set to 1 when we're in an endgame and thus doesn't encourage trading in the endgame itself.)
Acieved 80 wins, 30 draws and 68 losses against v0.6.1.

### Promote to make luft (v0.6.3)

Promote moves such as h3h2 more by tweaking the midgame pawn tables.

### Passed pawn bonus (v0.6.4)

Add a bonus for passed pawns. For each passed pawn we give 30 centipawns and an additional non linear bonus depending on how far up the board the pawn is (rank bonus). With the rank bonus being scaled also by how much we are in an endgame.
Against v0.6.3 this achieved 25 wins, 11 draws and 14 losses.

### isolated pawn penalty (v0.6.5)

We add a 15 centipawn penalty per isolated pawn.
This also disincentivizes isolated doubled pawns heavily.
Got 292 wins, 98 draws and 242 losses.

### Defended pawn bonus (v0.6.6)

We give small bonus to pawns when they are defended and also multiply the passed pawns score.
Got 214 wins, 67 draws and 191 losses

### Rooks on open file bonus (v0.6.7)

Give 10 centipawns for each rook on a half open file and 20 centipawns for each rook on a fully open file.
Got 141 wins, 27 draws and 90 losses.

### Rework king endgame behaviour (v0.6.8)

We tweaked the endgame square tables of the king to just uniformly discourage the king from being near the edge. We then also added a distance penalty for the distance to the nearest pawn. Lastly we added the square rule for passed pawns with a decent penalty for not being able to catch the pawn with the king and an very large penalty if there aren't other pieces that could catch the pawn (only king an pawn left for player that is catching the pawn).
Got 262 wins, 78 draws and 248 losses against v0.6.7.

### Bishop pair bonus (v0.6.9)

Add a 30 centipawn bonus for having 2 or more bishops. The bishop pair is a commonly known concept in high level chess.
Got 408 wins, 143 draws and 383 losses

### Total evaluation improvement

v0.6.9 achieved the following score against v0.5.0:
616 wins, 85 draws and 299 losses.

## Move ordering (v0.7.0)

If we order the moves from best to worst the engine can more effectively prune our search tree. We added a simple move ordering score which encourages first exploring queen promotions, then captures (first of more valuable pieces with less valuable pieces) and lastly the other moves.
This had the following effect on our performance when doing a full search untill depth 5 (and quiescent search after) on the positions in data/fens10.txt.

| Approach                                      | Average time (seconds) | Average nodes searched |
| --------------------------------------------- | ---------------------- | ---------------------- |
| No ordering                                   | 0.7422                 | 1,579,360              |
| Ordering in minimax                           | 0.0505                 | 77442                  |
| Ordering in both minimax and quiescent search | 0.0501                 | 77442                  |

The performance difference is significant (though sorting in quiescent search seems to have little impact). Note that the nodes per second does go down but we don't have to search as many nodes in return.
This Version is a massive improvement over v0.6.9 with 773 wins, 104 draws and 123 losses.

## Hash move (v0.7.1)

We can store the best (or cut-off producing) move in the transposition table entries. We can then order our moves such that we try this move first in the event that there is a TTEntry which is just not deep enough.
Doing so gave a slight increase in the depth we are able to reach (on 0.5 seconds think time almost always 1 depth deeper and sometimes even 2 deeper).

Again a large improvement against v0.7.0 with 466 wins, 181 draws and 223 losses.

## Use parial search results (v0.7.2)

Since we all but guarantee that the previous best move is searched first (except if we opt to not store the root result in the transposition table due to a collision). We can use the results even from the uncompleted searches. Since, either it didn't change our result or a better result than the best move was found in the partial search. Using these results got us 220 wins, 92 draws and 158 losses against v0.7.1, a significant improvement.

## Principle Variation Search (PVS) (v0.7.3)

We first refactored our search to the negamax approach instead of templating on bool Max. Which might have slightly boosted performance. To implement PVS we always search the first move (principle variation) fully and then only do a null window search on other moves to check if they need a full search. In some cases the bestEval in the search will be the result of a null window search. In these cases this only gives an upper bound even though the eval is between beta and alpha.
Decent improvement: 142 wins, 70 draws and 110 losses.

## In search repetition (v0.7.4)

Previously we were only comparing against positions from the game and not against positions reached during the search to determine repetitions. We now also detect repetitions against positions in the search tree itself.
Slight improvement: 417 wins, 195 draws and 388 losses.

## Move History (v0.7.5)

To improve the move ordering we track the moves (by the to square and piece type) and store a score based on wether they produced (early) cut-offs.
This improves the order the (quiet) moves are searched in which should allow for more pruning to be done.
From the testing I observed it could sometimes search one ply deeper than the previous version.
Minimal improvement: 161 wins, 87 draws, losses 142.

## Evaluation bug fixes (v0.7.6)

The evaluation function contained two bugs:

- White used blacks endgame tables (resulting in pawns becomming less valuable when they are pushed)
- In the endgameness score calculation the bonus material for queens used the white queen count for both black and white.
  Masive improvement: 283 wins, 65 draws and 130 losses

## History table tweaks (v0.7.7)

We make two adjustments to the history table. We split the history table into two (one for each color). Additionally we only add moves to the history table if they are quiet.
Decent improvement: 129 wins, 65 draws and 88 losses.

## TODO:

- move extensions
- multi threading

### evaluation

- space with pawns
- penalty for being stuck (for bishops/rooks)
- rework endgameness score
- not enough material detection
