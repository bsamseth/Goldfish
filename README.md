[![Build Status](https://travis-ci.org/bsamseth/Goldfish.svg?branch=master)](https://travis-ci.org/bsamseth/Goldfish)
[![Coverage Status](https://coveralls.io/repos/github/bsamseth/Goldfish/badge.svg?branch=master)](https://coveralls.io/github/bsamseth/Goldfish?branch=master)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/bsamseth/Goldfish/blob/master/LICENCE)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![Play link](https://img.shields.io/badge/Play%20Goldfish-lichess-green.svg)](https://lichess.org/@/Goldfish-Engine)

# Goldfish
###### Stockfish's very distant and not so bright cousin

### [Play Goldfish here](https://lichess.org/@/Goldfish-Engine)

## What is this?

This is a UCI chess engine. That means it is a program that can analyse chess
positions and propose best moves.  The program does not have its own UI, but
rather it implements the text based _Universal Chess Interface (UCI)_, which
makes it usable in most modern chess applications. This includes a live version
of Goldfish that you can play on [lichess.org](https://lichess.org/@/Goldfish-Engine). 

## Why is this?
This is not, nor does it attempt to be, the best chess engine out there.
It is was originally developed from scratch as a
practice project for learning C++. The end vision at that point was to have a
working chess engine with a UCI interface. In 2018 the engine was playable, and
somewhat stable. However, it was bad. Really bad. At this point it was decided
to abandon my initial work instead of trying to work around bad design
decisions made by the former me. 

The current version is based on
[fluxroot/pulse](https://github.com/fluxroot/pulse). Thanks a lot to the
original author for his great work. Starting with this project meant a stable
starting point, with all the rules sorted out, basic search in place and a test
suite to make sure it all works as expected.

## Why Goldfish?

For some reason, several top chess engines have names of different fish, e.g.
Stockfish, Rybka and others. Goldfish are known for their very limited memory,
and so it seemed only fitting for my somewhat limited program to be named this. 

## Road map

The current plan for the project is to improve the strength, including, but not
limited to:

- [X] Making the engine playable on [lichess.org](lichess.org)
- [X] Complete refactoring of base types
- [X] Null move pruning
- [ ] Transposition table
- [X] Check extensions
- [ ] Passed pawn
- [ ] Staged move generation
- [ ] Better search algorithms, such as MTD-bi
- [ ] More sophisticated static evaluation

Each significant change will result in a new version of the engine (see
releases). In the following you see a relative rating between the current
versions. In this rating system, v1.0 is held at 2000 rating points, and the
others are adjusted accordingly. This gives an impression of the relative
improvements of the engine over time.

```text
   # PLAYER           :  RATING  POINTS  PLAYED   (%)
   1 Goldfish v1.4    :  2132.1    55.0     100    55
   2 Goldfish v1.3    :  2110.6    95.5     180    53
   3 Goldfish v1.2    :  2082.6    74.5     160    47
   4 Goldfish v1.1    :  2055.7    53.5     112    48
   5 Goldfish v1.0    :  2000.0    13.5      32    42

Head to head statistics:

1) Goldfish v1.4 2132.1 :    100 (+26,=58,-16),  55.0 %

   vs.                  :  games (  +,  =,  -),   (%) :    Diff
   Goldfish v1.3        :     50 ( 11, 30,  9),  52.0 :   +21.5
   Goldfish v1.2        :     50 ( 15, 28,  7),  58.0 :   +49.5

2) Goldfish v1.3 2110.6 :    180 (+38,=115,-27),  53.1 %

   vs.                  :  games (  +,   =,  -),   (%) :    Diff
   Goldfish v1.4        :     50 (  9,  30, 11),  48.0 :   -21.5
   Goldfish v1.2        :     80 ( 16,  55,  9),  54.4 :   +28.0
   Goldfish v1.1        :     50 ( 13,  30,  7),  56.0 :   +55.0

3) Goldfish v1.2 2082.6 :    160 (+24,=101,-35),  46.6 %

   vs.                  :  games (  +,   =,  -),   (%) :    Diff
   Goldfish v1.4        :     50 (  7,  28, 15),  42.0 :   -49.5
   Goldfish v1.3        :     80 (  9,  55, 16),  45.6 :   -28.0
   Goldfish v1.1        :     30 (  8,  18,  4),  56.7 :   +27.0

4) Goldfish v1.1 2055.7 :    112 (+18,=71,-23),  47.8 %

   vs.                  :  games (  +,  =,  -),   (%) :    Diff
   Goldfish v1.3        :     50 (  7, 30, 13),  44.0 :   -55.0
   Goldfish v1.2        :     30 (  4, 18,  8),  43.3 :   -27.0
   Goldfish v1.0        :     32 (  7, 23,  2),  57.8 :   +55.7

5) Goldfish v1.0 2000.0 :     32 (+2,=23,-7),  42.2 %

   vs.                  :  games ( +,  =, -),   (%) :    Diff
   Goldfish v1.1        :     32 ( 2, 23, 7),  42.2 :   -55.7
```

This is meant as a project to work on as practice/just for the fun of it.
Contributions are very welcome if you feel like it.

## Build

The project is written in C++17. It can be built using CMake, which hopefully makes this as portable as
possible. Recommend building in a separate directory:

``` bash
$ mkdir build && cd build
$ export CXX=g++-8  # Or some C++17 compliant compiler of choice.
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

After the compiling is done you should have two executables in the build
directory: `goldfish.x` and `unit_tests.x`. The former is the interface to the
engine it self.

## Run

After building the `goldfish.x` executable, you _can_ run it directly. The engine
speaks using the UCI protocol. An example (Scid is the UI communicating with the engine, or you if you run it directly):

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
as Scid. Or better still, play on [lichess.org](https://lichess.org/@/Goldfish-Engine).
