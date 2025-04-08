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
This addition resulted in 465 wins, 162 draws and 373 wins against the previous version (~40 elo increase). Though we suspect that the strength increase is larger in slower time controls as with only 0.5 seconds for each move the depth of the entries in the tables is never verry high.
