# ChessBitBoards

An implementation of the game of chess using `bitboards` + an engine (both written in c++).

## Building

Make a build (in ChessBitBoards/) directory:

`mkdir build`

Go into the build directory:

`cd build`

Then run:

`cmake .. && make`

## Playing against the engine

To play the engine you have a few options. After the build process you should have in the app folder an executable named `engine`. Running starts a command line interface (see commands [here](/docs/engineCommands.md)). Another option is to run `play.py` which is at the root of this github repository. This is a wrapper around the `engine` executable which adds a GUI to render the board, but still requires uci moves to be entered in the console. Optionally you can also provide the path to another executable which supports the same commands to play.py.

## Testing engine versions
To test engine versions against eachother the script `testing/enginePlayout.py` is used.
To use this file one needs to have the releases that need to be tested in `releases/` (one can make this folder if you don't have it yet). Then download the latest release, 