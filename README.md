[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![Build Status](https://travis-ci.org/bsamseth/Goldfish.svg?branch=master)](https://travis-ci.org/bsamseth/Goldfish)
[![Coverage Status](https://coveralls.io/repos/github/bsamseth/Goldfish/badge.svg?branch=master)](https://coveralls.io/github/bsamseth/Goldfish?branch=master)
[![codecov](https://codecov.io/gh/bsamseth/Goldfish/branch/master/graph/badge.svg)](https://codecov.io/gh/bsamseth/Goldfish)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7f53976fd4bb42b4bfb2f53bd67fce65)](https://www.codacy.com/app/bsamseth/Goldfish?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=bsamseth/Goldfish&amp;utm_campaign=Badge_Grade)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/bsamseth/Goldfish.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bsamseth/Goldfish/context:cpp)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/bsamseth/Goldfish.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bsamseth/Goldfish/context:python)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/bsamseth/Goldfish.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bsamseth/Goldfish/alerts/)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/bsamseth/Goldfish/blob/master/LICENCE)
[![Play link](https://img.shields.io/badge/Play%20Goldfish-lichess-green.svg)](https://lichess.org/@/Goldfish-Engine)

# Goldfish
> Stockfish's very distant and not so bright cousin

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

At this point, the newest version of Goldfish has been substantially revamped and improved 
in most aspects (see [Road map](#road-map)).  

## Why Goldfish?

For some reason, several top chess engines have names of different fish, e.g.
Stockfish, Rybka and others. Goldfish are known for their very limited memory,
and so it seemed only fitting for my somewhat limited program to be named this. 

## Road map

The current plan for the project is to improve the strength. The following is a
non-exhaustive list of possibilities for future additions, including all features that have
been added so far. The list is inspired in large part by [this writeup](http://www.frayn.net/beowulf/theory.html).

-   [X] Making the engine playable on [lichess.org](lichess.org)
-   [X] Complete refactoring of base types
-   [X] Null move pruning
-   [X] Transposition table
-   [X] Check extensions
-   [X] Killer move heuristic
-   [X] Principal variation search
-   [X] Internal iterative deepening
-   [X] Aspiration window search
-   [ ] Futility pruning
-   [ ] Delta pruning in quiescence search.
    +   [X] Prune when _no_ move can improve enough
    +   [ ] Prune captures that are insufficient to improve
-   [ ] Staged move generation
-   [ ] Better search algorithms, such as MTD-bi (?)
-   [ ] More sophisticated static evaluation
    +   [ ] Extra considerations for passed pawns
    +   [ ] Piece square tables
    +   [ ] King safety
    +   [ ] Center control
    +   [ ] Rooks on the 7th rank
    + [ ] Bishops on main diagonals

Each significant change will result in a new version of the engine (see
[releases](https://github.com/bsamseth/Goldfish/releases)). In the following
you see a relative rating between the current versions. In this rating system,
v1.0 is held at 2000 rating points, and the others are adjusted accordingly.
This gives an impression of the relative improvements of the engine over time.

   |#| PLAYER             |  RATING  |POINTS  |PLAYED|   (%)|
   |:---:|---|:---:|:---:|:---:|:---:|
   |1| Goldfish v1.7.0    |  2251.5   |219.5     |370    |59|
   |2| Goldfish v1.6.0    |  2219.2   |399.0     |720    |55|
   |3| Goldfish v1.5.1    |  2169.1   |460.5     |970    |47|
   |4| Goldfish v1.5      |  2158.9   |554.5    |1145    |48|
   |5| Goldfish v1.4      |  2153.8   |487.0     |970    |50|
   |6| Goldfish v1.3      |  2131.0   |162.5     |325    |50|
   |7| Goldfish v1.2      |  2112.4   |107.5     |230    |47|
   |8| Goldfish v1.1      |  2055.3    |93.0     |232    |40|
   |9| Goldfish v1.0      |  2000.0    |13.5      |32    |42|


```text
Head to head statistics:

 1) Goldfish v1.7.0 2251.5 :    370 (+115,=209,-46),  59.3 %

    vs.                    :  games (   +,   =,  -),   (%) :    Diff
    Goldfish v1.6.0        :    160 (  47,  85, 28),  55.9 :   +32.3
    Goldfish v1.5.1        :    160 (  43, 102, 15),  58.8 :   +82.4
    Goldfish v1.5          :     10 (   4,   5,  1),  65.0 :   +92.6
    Goldfish v1.4          :     10 (   4,   6,  0),  70.0 :   +97.7
    Goldfish v1.3          :     10 (   5,   5,  0),  75.0 :  +120.5
    Goldfish v1.2          :     10 (   4,   4,  2),  60.0 :  +139.1
    Goldfish v1.1          :     10 (   8,   2,  0),  90.0 :  +196.2

 2) Goldfish v1.6.0 2219.2 :    720 (+180,=438,-102),  55.4 %

    vs.                    :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0        :    160 (  28,  85,  47),  44.1 :   -32.3
    Goldfish v1.5.1        :    260 (  66, 162,  32),  56.5 :   +50.1
    Goldfish v1.5          :    260 (  77, 163,  20),  61.0 :   +60.3
    Goldfish v1.4          :     10 (   1,   8,   1),  50.0 :   +65.4
    Goldfish v1.3          :     10 (   1,   8,   1),  50.0 :   +88.2
    Goldfish v1.2          :     10 (   4,   6,   0),  70.0 :  +106.8
    Goldfish v1.1          :     10 (   3,   6,   1),  60.0 :  +163.9

 3) Goldfish v1.5.1 2169.1 :    970 (+145,=631,-194),  47.5 %

    vs.                    :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0        :    160 (  15, 102,  43),  41.2 :   -82.4
    Goldfish v1.6.0        :    260 (  32, 162,  66),  43.5 :   -50.1
    Goldfish v1.5          :    260 (  45, 172,  43),  50.4 :   +10.2
    Goldfish v1.4          :    260 (  45, 176,  39),  51.2 :   +15.3
    Goldfish v1.3          :     10 (   2,   7,   1),  55.0 :   +38.1
    Goldfish v1.2          :     10 (   2,   6,   2),  50.0 :   +56.7
    Goldfish v1.1          :     10 (   4,   6,   0),  70.0 :  +113.8

 4) Goldfish v1.5   2158.9 :   1145 (+174,=761,-210),  48.4 %

    vs.                    :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0        :     10 (   1,   5,   4),  35.0 :   -92.6
    Goldfish v1.6.0        :    260 (  20, 163,  77),  39.0 :   -60.3
    Goldfish v1.5.1        :    260 (  43, 172,  45),  49.6 :   -10.2
    Goldfish v1.4          :    510 (  88, 352,  70),  51.8 :    +5.1
    Goldfish v1.3          :     85 (  12,  61,  12),  50.0 :   +27.9
    Goldfish v1.2          :     10 (   4,   6,   0),  70.0 :   +46.5
    Goldfish v1.1          :     10 (   6,   2,   2),  70.0 :  +103.5

 5) Goldfish v1.4   2153.8 :    970 (+164,=646,-160),  50.2 %

    vs.                    :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0        :     10 (   0,   6,   4),  30.0 :   -97.7
    Goldfish v1.6.0        :     10 (   1,   8,   1),  50.0 :   -65.4
    Goldfish v1.5.1        :    260 (  39, 176,  45),  48.8 :   -15.3
    Goldfish v1.5          :    510 (  70, 352,  88),  48.2 :    -5.1
    Goldfish v1.3          :     60 (  13,  37,  10),  52.5 :   +22.8
    Goldfish v1.2          :     60 (  17,  34,   9),  56.7 :   +41.4
    Goldfish v1.1          :     60 (  24,  33,   3),  67.5 :   +98.4

 6) Goldfish v1.3   2131.0 :    325 (+55,=215,-55),  50.0 %

    vs.                    :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0        :     10 (  0,   5,  5),  25.0 :  -120.5
    Goldfish v1.6.0        :     10 (  1,   8,  1),  50.0 :   -88.2
    Goldfish v1.5.1        :     10 (  1,   7,  2),  45.0 :   -38.1
    Goldfish v1.5          :     85 ( 12,  61, 12),  50.0 :   -27.9
    Goldfish v1.4          :     60 ( 10,  37, 13),  47.5 :   -22.8
    Goldfish v1.2          :     90 ( 17,  61, 12),  52.8 :   +18.6
    Goldfish v1.1          :     60 ( 14,  36, 10),  53.3 :   +75.7

 7) Goldfish v1.2   2112.4 :    230 (+37,=141,-52),  46.7 %

    vs.                    :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0        :     10 (  2,   4,  4),  40.0 :  -139.1
    Goldfish v1.6.0        :     10 (  0,   6,  4),  30.0 :  -106.8
    Goldfish v1.5.1        :     10 (  2,   6,  2),  50.0 :   -56.7
    Goldfish v1.5          :     10 (  0,   6,  4),  30.0 :   -46.5
    Goldfish v1.4          :     60 (  9,  34, 17),  43.3 :   -41.4
    Goldfish v1.3          :     90 ( 12,  61, 17),  47.2 :   -18.6
    Goldfish v1.1          :     40 ( 12,  24,  4),  60.0 :   +57.1

 8) Goldfish v1.1   2055.3 :    232 (+27,=132,-73),  40.1 %

    vs.                    :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0        :     10 (  0,   2,  8),  10.0 :  -196.2
    Goldfish v1.6.0        :     10 (  1,   6,  3),  40.0 :  -163.9
    Goldfish v1.5.1        :     10 (  0,   6,  4),  30.0 :  -113.8
    Goldfish v1.5          :     10 (  2,   2,  6),  30.0 :  -103.5
    Goldfish v1.4          :     60 (  3,  33, 24),  32.5 :   -98.4
    Goldfish v1.3          :     60 ( 10,  36, 14),  46.7 :   -75.7
    Goldfish v1.2          :     40 (  4,  24, 12),  40.0 :   -57.1
    Goldfish v1.0          :     32 (  7,  23,  2),  57.8 :   +55.3

 9) Goldfish v1.0   2000.0 :     32 (+2,=23,-7),  42.2 %

    vs.                    :  games ( +,  =, -),   (%) :    Diff
    Goldfish v1.1          :     32 ( 2, 23, 7),  42.2 :   -55.3

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
