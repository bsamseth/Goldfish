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
-   [X] Syzygy endgame tablebases
-   [X] Check extensions
-   [X] Killer move heuristic
-   [X] Principal variation search
-   [X] Internal iterative deepening
-   [X] Aspiration window search (repealed by PR #27, needs tuning before reapplying)
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


   |  # | PLAYER              |  RATING | POINTS | PLAYED | (%) |
   |:---:|---|:---:|:---:|:---:|:---:|
   |  1 | Goldfish v1.9.0     |  2353.2 |  831.0 |   1530 |  54|
   |  2 | Goldfish v1.11.0    |  2352.9 |  549.5 |   1100 |  50|
   |  3 | Goldfish v1.7.0     |  2251.6 |  877.5 |   1699 |  52|
   |  4 | Goldfish v1.7.1     |  2243.8 |  244.0 |    477 |  51|
   |  5 | Goldfish v1.8.0     |  2237.1 |  324.0 |    650 |  50|
   |  6 | Goldfish v1.8.1     |  2234.2 |  485.5 |   1000 |  49|
   |  7 | Goldfish v1.8.2     |  2234.1 |  174.0 |    429 |  41|
   |  8 | Goldfish v1.6.0     |  2218.8 |  434.5 |    797 |  55|
   |  9 | Goldfish v1.7.2     |  2218.1 |   69.5 |    150 |  46|
   | 10 | Goldfish v1.5.1     |  2168.9 |  460.5 |    970 |  47|
   | 11 | Goldfish v1.5       |  2158.6 |  554.5 |   1145 |  48|
   | 12 | Goldfish v1.4       |  2153.6 |  487.0 |    970 |  50|
   | 13 | Goldfish v1.3       |  2130.8 |  162.5 |    325 |  50|
   | 14 | Goldfish v1.2       |  2112.2 |  107.5 |    230 |  47|
   | 15 | Goldfish v1.1       |  2055.2 |   93.0 |    232 |  40|
   | 16 | Goldfish v1.0       |  2000.0 |   13.5 |     32 |  42|

```text
Head to head statistics:

 1) Goldfish v1.9.0  2353.2 :   1530 (+573,=516,-441),  54.3 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.11.0        :   1100 ( 372, 357, 371),  50.0 :    +0.3
    Goldfish v1.7.0         :    215 ( 101,  84,  30),  66.5 :  +101.6
    Goldfish v1.8.2         :    215 ( 100,  75,  40),  64.0 :  +119.1

 2) Goldfish v1.11.0 2352.9 :   1100 (+371,=357,-372),  50.0 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.9.0         :   1100 ( 371, 357, 372),  50.0 :    -0.3

 3) Goldfish v1.7.0  2251.6 :   1699 (+383,=989,-327),  51.6 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.9.0         :    215 (  30,  84, 101),  33.5 :  -101.6
    Goldfish v1.7.1         :    250 (  44, 168,  38),  51.2 :    +7.7
    Goldfish v1.8.0         :    150 (  32,  84,  34),  49.3 :   +14.5
    Goldfish v1.8.1         :    500 ( 114, 305,  81),  53.3 :   +17.4
    Goldfish v1.8.2         :    214 (  48, 139,  27),  54.9 :   +17.5
    Goldfish v1.6.0         :    160 (  47,  85,  28),  55.9 :   +32.8
    Goldfish v1.5.1         :    160 (  43, 102,  15),  58.8 :   +82.7
    Goldfish v1.5           :     10 (   4,   5,   1),  65.0 :   +93.0
    Goldfish v1.4           :     10 (   4,   6,   0),  70.0 :   +98.0
    Goldfish v1.3           :     10 (   5,   5,   0),  75.0 :  +120.8
    Goldfish v1.2           :     10 (   4,   4,   2),  60.0 :  +139.4
    Goldfish v1.1           :     10 (   8,   2,   0),  90.0 :  +196.4

 4) Goldfish v1.7.1  2243.8 :    477 (+97,=294,-86),  51.2 %

    vs.                     :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0         :    250 ( 38, 168, 44),  48.8 :    -7.7
    Goldfish v1.6.0         :     77 ( 19,  45, 13),  53.9 :   +25.1
    Goldfish v1.7.2         :    150 ( 40,  81, 29),  53.7 :   +25.8

 5) Goldfish v1.8.0  2237.1 :    650 (+133,=382,-135),  49.8 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :    150 (  34,  84,  32),  50.7 :   -14.5
    Goldfish v1.8.1         :    500 (  99, 298, 103),  49.6 :    +2.9

 6) Goldfish v1.8.1  2234.2 :   1000 (+184,=603,-213),  48.5 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :    500 (  81, 305, 114),  46.7 :   -17.4
    Goldfish v1.8.0         :    500 ( 103, 298,  99),  50.4 :    -2.9

 7) Goldfish v1.8.2  2234.1 :    429 (+67,=214,-148),  40.6 %

    vs.                     :  games (  +,   =,   -),   (%) :    Diff
    Goldfish v1.9.0         :    215 ( 40,  75, 100),  36.0 :  -119.1
    Goldfish v1.7.0         :    214 ( 27, 139,  48),  45.1 :   -17.5

 8) Goldfish v1.6.0  2218.8 :    797 (+193,=483,-121),  54.5 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :    160 (  28,  85,  47),  44.1 :   -32.8
    Goldfish v1.7.1         :     77 (  13,  45,  19),  46.1 :   -25.1
    Goldfish v1.5.1         :    260 (  66, 162,  32),  56.5 :   +49.9
    Goldfish v1.5           :    260 (  77, 163,  20),  61.0 :   +60.1
    Goldfish v1.4           :     10 (   1,   8,   1),  50.0 :   +65.2
    Goldfish v1.3           :     10 (   1,   8,   1),  50.0 :   +88.0
    Goldfish v1.2           :     10 (   4,   6,   0),  70.0 :  +106.5
    Goldfish v1.1           :     10 (   3,   6,   1),  60.0 :  +163.5

 9) Goldfish v1.7.2  2218.1 :    150 (+29,=81,-40),  46.3 %

    vs.                     :  games (  +,  =,  -),   (%) :    Diff
    Goldfish v1.7.1         :    150 ( 29, 81, 40),  46.3 :   -25.8

10) Goldfish v1.5.1  2168.9 :    970 (+145,=631,-194),  47.5 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :    160 (  15, 102,  43),  41.2 :   -82.7
    Goldfish v1.6.0         :    260 (  32, 162,  66),  43.5 :   -49.9
    Goldfish v1.5           :    260 (  45, 172,  43),  50.4 :   +10.3
    Goldfish v1.4           :    260 (  45, 176,  39),  51.2 :   +15.3
    Goldfish v1.3           :     10 (   2,   7,   1),  55.0 :   +38.1
    Goldfish v1.2           :     10 (   2,   6,   2),  50.0 :   +56.7
    Goldfish v1.1           :     10 (   4,   6,   0),  70.0 :  +113.6

11) Goldfish v1.5    2158.6 :   1145 (+174,=761,-210),  48.4 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :     10 (   1,   5,   4),  35.0 :   -93.0
    Goldfish v1.6.0         :    260 (  20, 163,  77),  39.0 :   -60.1
    Goldfish v1.5.1         :    260 (  43, 172,  45),  49.6 :   -10.3
    Goldfish v1.4           :    510 (  88, 352,  70),  51.8 :    +5.1
    Goldfish v1.3           :     85 (  12,  61,  12),  50.0 :   +27.8
    Goldfish v1.2           :     10 (   4,   6,   0),  70.0 :   +46.4
    Goldfish v1.1           :     10 (   6,   2,   2),  70.0 :  +103.4

12) Goldfish v1.4    2153.6 :    970 (+164,=646,-160),  50.2 %

    vs.                     :  games (   +,   =,   -),   (%) :    Diff
    Goldfish v1.7.0         :     10 (   0,   6,   4),  30.0 :   -98.0
    Goldfish v1.6.0         :     10 (   1,   8,   1),  50.0 :   -65.2
    Goldfish v1.5.1         :    260 (  39, 176,  45),  48.8 :   -15.3
    Goldfish v1.5           :    510 (  70, 352,  88),  48.2 :    -5.1
    Goldfish v1.3           :     60 (  13,  37,  10),  52.5 :   +22.7
    Goldfish v1.2           :     60 (  17,  34,   9),  56.7 :   +41.3
    Goldfish v1.1           :     60 (  24,  33,   3),  67.5 :   +98.3

13) Goldfish v1.3    2130.8 :    325 (+55,=215,-55),  50.0 %

    vs.                     :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0         :     10 (  0,   5,  5),  25.0 :  -120.8
    Goldfish v1.6.0         :     10 (  1,   8,  1),  50.0 :   -88.0
    Goldfish v1.5.1         :     10 (  1,   7,  2),  45.0 :   -38.1
    Goldfish v1.5           :     85 ( 12,  61, 12),  50.0 :   -27.8
    Goldfish v1.4           :     60 ( 10,  37, 13),  47.5 :   -22.7
    Goldfish v1.2           :     90 ( 17,  61, 12),  52.8 :   +18.6
    Goldfish v1.1           :     60 ( 14,  36, 10),  53.3 :   +75.6

14) Goldfish v1.2    2112.2 :    230 (+37,=141,-52),  46.7 %

    vs.                     :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0         :     10 (  2,   4,  4),  40.0 :  -139.4
    Goldfish v1.6.0         :     10 (  0,   6,  4),  30.0 :  -106.5
    Goldfish v1.5.1         :     10 (  2,   6,  2),  50.0 :   -56.7
    Goldfish v1.5           :     10 (  0,   6,  4),  30.0 :   -46.4
    Goldfish v1.4           :     60 (  9,  34, 17),  43.3 :   -41.3
    Goldfish v1.3           :     90 ( 12,  61, 17),  47.2 :   -18.6
    Goldfish v1.1           :     40 ( 12,  24,  4),  60.0 :   +57.0

15) Goldfish v1.1    2055.2 :    232 (+27,=132,-73),  40.1 %

    vs.                     :  games (  +,   =,  -),   (%) :    Diff
    Goldfish v1.7.0         :     10 (  0,   2,  8),  10.0 :  -196.4
    Goldfish v1.6.0         :     10 (  1,   6,  3),  40.0 :  -163.5
    Goldfish v1.5.1         :     10 (  0,   6,  4),  30.0 :  -113.6
    Goldfish v1.5           :     10 (  2,   2,  6),  30.0 :  -103.4
    Goldfish v1.4           :     60 (  3,  33, 24),  32.5 :   -98.3
    Goldfish v1.3           :     60 ( 10,  36, 14),  46.7 :   -75.6
    Goldfish v1.2           :     40 (  4,  24, 12),  40.0 :   -57.0
    Goldfish v1.0           :     32 (  7,  23,  2),  57.8 :   +55.2

16) Goldfish v1.0    2000.0 :     32 (+2,=23,-7),  42.2 %

    vs.                     :  games ( +,  =, -),   (%) :    Diff
    Goldfish v1.1           :     32 ( 2, 23, 7),  42.2 :   -55.2

Total games                5869
 - White wins              1383
 - Draws                   3234
 - Black wins              1251
 - Truncated/Discarded        1
Unique head to head        1.33%
Reference rating      2000.0 (set to "Goldfish v1.0")

players with no games = 0
players with all wins = 0
players w/ all losses = 0
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

Although it is possible to use the text based interface directly, it's
recommended to run this through a UCI compatible graphical user interface, such
as Scid. Or better still, play on [lichess.org](https://lichess.org/@/Goldfish-Engine).

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

