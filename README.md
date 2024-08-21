[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/bsamseth/Goldfish/blob/main/LICENCE)
[![Play link](https://img.shields.io/badge/Play%20Goldfish-lichess-green.svg)](https://lichess.org/@/Goldfish-Engine)



# Goldfish
> Stockfish's very distant and not so bright cousin

### [Play Goldfish here](https://lichess.org/@/Goldfish-Engine)

## What is this?

This is a [UCI](https://www.wbec-ridderkerk.nl/html/UCIProtocol.html) chess engine. 
That means it is a program that can analyse chess
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

I then made a new version based on
[fluxroot/pulse](https://github.com/fluxroot/pulse). Thanks a lot to the
original author for his great work. From there, Goldfish was substantially
revamped and improved in most aspects, leaving little evidence of it's starting
point. [Version 1.13.0](https://github.com/bsamseth/Goldfish/releases/tag/v1.13.0) 
is the final version of the C++ version of Goldfish.

In late 2023 I wanted to get better at Rust, and started a complete rewrite. By
mid 2024, the new 2.0 version exceeded version 1.13 in playing strength. The
two version share in their overall search logic, but the Rust version is at
this point more sophisticated.

## Why Goldfish?

For some reason, several top chess engines have names of different fish, e.g.
Stockfish, Rybka and others. Goldfish are known for their very limited memory,
and so it seemed only fitting for my somewhat limited program to be named this. 

## Road map

I mostly enjoy improving the search algorithm more so than the evaluation function.
Because of this, the evaluation function is currently _very_ simple. In fact, it is exactly the 
[Simplified Evaluation function](https://www.chessprogramming.org/Simplified_Evaluation_Function) 
from the Chess Programming Wiki. Just a material count and piece square tables. Nothing more. For now at least.

The current plan for the project is to improve the strength. The following is a
non-exhaustive list of possibilities for future additions, including all features that have
been added so far. The list is inspired in large part by [this writeup](http://www.frayn.net/beowulf/theory.html).

-   [X] Making the engine playable on [lichess.org](lichess.org)
-   [X] Null move pruning
-   [X] Transposition table
-   [X] Syzygy endgame tablebases
-   [X] Check extensions
-   [X] Killer move heuristic
-   [X] Principal variation search
-   [X] Internal iterative deepening
-   [X] Futility pruning
-   [X] Razoring
-   [X] History heuristic
-   [X] Delta pruning in quiescence search.
    +   [X] Prune when _no_ move can improve enough
    +   [X] Prune captures that are insufficient to improve
-   [ ] Tapered evaluation
-   [ ] Aspiration window search
-   [ ] Late move reductions
-   [ ] Static exchange evaluation for move ordering
-   [ ] More quiet move ordering heuristics
-   [ ] ProbCut
-   etc.

Each significant change will result in a new version of the engine (see
[releases](https://github.com/bsamseth/Goldfish/releases)). In the following
you see a relative rating between the current versions, based on test matches
played with a variety of (short) time controls. In this rating system, v1.13 is
held at its [CCRL 40/40](http://computerchess.org.uk/ccrl/4040/) rating of, and
the others are adjusted accordingly. This gives an impression of the relative
improvements of the engine over time, but cannot be compared directly to any
other rating systems (e.g. FIDE).

 |   # | PLAYER               |   RATING  | POINTS  | PLAYED   | (%)|
   |:---:|---|:---:|:---:|:---:|:---:|
 |  1 | Goldfish v2.0.0-rc7    |  2050.6  |  430.0  |  650  | 66.2% |
 |  2 | Goldfish v1.13.0       |  1983.0  | 1107.5  | 1942  | 57.0% |
 |  3 | Goldfish v1.12.1       |  1932.1  |  748.0  | 1613  | 46.4% |
 |  4 | Goldfish v1.12.0       |  1906.7  |  833.5  | 1528  | 54.5% |
 |  5 | Goldfish v1.11.1       |  1893.5  | 1095.0  | 2015  | 54.3% |
 |  6 | Goldfish v1.9.0        |  1878.8  | 1826.5  | 3538  | 51.6% |
 |  7 | Goldfish v1.11.0       |  1878.5  |  549.5  | 1100  | 50.0% |
 |  8 | Goldfish v2.0.0-rc6    |  1871.1  |  136.0  |  338  | 40.2% |
 |  9 | Goldfish v2.0.0-rc3    |  1862.1  |  177.5  |  417  | 42.6% |
 | 10 | Goldfish v2.0.0-rc5    |  1858.8  |  120.5  |  316  | 38.1% |
 | 11 | Goldfish v2.0.0-rc1    |  1837.3  |  217.5  |  436  | 49.9% |
 | 12 | Goldfish v2.0.0-rc2    |  1835.0  |  141.0  |  354  | 39.8% |
 | 13 | Goldfish v1.7.0        |  1814.3  | 1086.5  | 2053  | 52.9% |
 | 14 | Goldfish v1.8.2        |  1809.4  |  392.0  |  783  | 50.1% |
 | 15 | Goldfish v1.7.1        |  1807.2  |  244.0  |  477  | 51.2% |
 | 16 | Goldfish v2.0.0-rc4    |  1803.7  |   19.5  |   57  | 34.2% |
 | 17 | Goldfish v1.8.0        |  1799.8  |  324.0  |  650  | 49.8% |
 | 18 | Goldfish v1.8.1        |  1796.8  |  485.5  | 1000  | 48.5% |
 | 19 | Goldfish v1.6.0        |  1784.4  |  625.0  | 1151  | 54.3% |
 | 20 | Goldfish v1.7.2        |  1781.4  |   69.5  |  150  | 46.3% |
 | 21 | Goldfish v1.5.1        |  1726.7  |  645.5  | 1435  | 45.0% |
 | 22 | Goldfish v1.5          |  1724.4  |  554.5  | 1145  | 48.4% |
 | 23 | Goldfish v1.4          |  1721.5  |  646.5  | 1325  | 48.8% |
 | 24 | Goldfish v1.3          |  1706.8  |  314.0  |  680  | 46.2% |
 | 25 | Goldfish v1.2          |  1676.3  |  237.5  |  585  | 40.6% |
 | 26 | Goldfish v1.1          |  1635.1  |  210.0  |  597  | 35.2% |
 | 27 | Goldfish v1.0          |  1629.8  |  129.5  |  397  | 32.6% |



Detailed head-to-head statistics can be found [here](stats/head-to-head-history.txt).

## Installation

Each release comes with pre-compiled executables for Linux and Windows. Just download these and run them in your GUI of
choice.

## Build

Requires a recent Rust toolchain, with no particular MSRV. Get the latest stable version from [rustup](https://rustup.rs/).

``` bash
cd src
cargo build --profile release-lto --package engine
```

This resulting executable will be in `src/target/release-lto/goldfish[.exe]`.
