# Engine commands

The engine class has the function `runCmd` which is used to run all commands provided by the user. The following is a list of all the available commands.

## getPosition

`getPosition` has no arguments and returns the fen of the current board.

## setPosition

`setPosition [fen]` sets the position to the specified fen.

## bestMove

`bestMove [thinkTime]` lets the engine evaluate the position to find the best move.
Not that the thinkTime is in seconds.

## makeMove

`makeMove [uciMove]` makes the specified move on the board. The provided move should be a string of the move in uci format.

## quit

`quit` stops the engine.

## bench

`bench [type] [quantity]`. Currently bench supports `depth` and returns the time it took.

## go

Usage can be either `go wtime [ms] btime [ms]` or `go wtime [ms] btime [ms] winc [ms] binc [ms]`
