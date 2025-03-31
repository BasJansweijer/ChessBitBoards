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
Doing so improved the performance by roughly 250 elo. The results against the previous version were 449 wins, 435 draws and 116 losses.

## Repitition detection (v0.4.0)

We add zobrist hashing and utilize a transposition table to detect when previous positions are repeated.

### 50 move rule (v0.4.1)

Performs almost identically to to v0.4.0 but adds detection of 50 move rule. Initially it also added a component to the eval which scaled down the score as we got closer to the 50 move rule draw, but this was removed as it decouples the evaluation from the pure board state which would make the transposition table evaluations unreliable once this term starts playing a role. (It lost 224 times, won 221 times and drew 107 times against v0.4.0).

## Transposition table (v0.5.0)
