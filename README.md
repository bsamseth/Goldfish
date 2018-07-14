[![Build Status](https://travis-ci.org/bsamseth/Goldfish.svg?branch=master)](https://travis-ci.org/bsamseth/Goldfish)
[![Coverage Status](https://coveralls.io/repos/github/bsamseth/Goldfish/badge.svg?branch=master)](https://coveralls.io/github/bsamseth/Goldfish?branch=master)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/bsamseth/Goldfish/blob/master/LICENCE)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![Play link](https://img.shields.io/badge/Play%20Goldfish-lichess-green.svg)](https://lichess.org/@/Goldfish-Engine)

# Goldfish
###### Stockfish's very distant and not so bright cousin

### [Play Goldfish](https://lichess.org/@/Goldfish-Engine)

This is a UCI chess engine. It is was originally developed from scratch as a
practice project for learning C++. The end vision at that point was to have a
working chess engine with a UCI interface. In 2018 the engine was playable, and
somewhat stable. However, it was bad. Really bad. At this point it was decided
to abandon my initial work instead of trying to work around bad design
decisions made by the former me. 

The current version is based heavily on
[fluxroot/pulse](https://github.com/fluxroot/pulse). Thanks a lot to the
original author for his great work. Starting with this project meant a stable
starting point, with all the rules sorted out, basic search in place and a test
suite to make sure it all works as expected.

As suggested in [fluxroot/pulse](https://github.com/fluxroot/pulse), the
current plan for the project is to improve the strength, including, but not
limited to:

- [X] Null move pruning
- [ ] Transposition table
- [X] Check extensions
- [ ] Passed pawn
- [ ] Staged move generation
- [ ] Better search algorithms, such as MTD-bi
- [ ] More sophisticated static evaluation
- [X] Making the engine playable on [lichess.org](lichess.org)

This is meant as a project to work on as practice/just for the fun of it.
Contributions are welcome if you feel like it.

## Build

The project is built using CMake, which hopefully makes this as portable as
possible. Recommend building in a separate directory:

``` bash
$ mkdir build && cd build
$ cmake ..
$ make
```

After the compiling is done you should have two executables in the build
directory: `goldfish` and `unit_tests.x`. The former is the interface to the
engine it self.

## Run

After building the `goldfish` executable, you _can_ run it directly. The engine
speaks using the UCI protocol. An example from the conversation between Scid
and the engine:

``` text
Scid  : uci
Engine: id name Goldfish
Engine: id author Bendik Samseth
Engine: uciok
Scid  : isready
Engine: readyok
Scid  : position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 1 9
Scid  : go infinite
Engine: info depth 1 score cp -1 time 10 nodes 26 nps 633 pv d7d6
Engine: bestmove d7d6
```

Although it is possible to use the text based interface directly, it's
recommended to run this through a UCI compatible graphical user interface, such
as Scid.

