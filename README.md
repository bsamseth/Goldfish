[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)
[![Build and test](https://github.com/bsamseth/Goldfish/actions/workflows/cargo-build-test.yml/badge.svg)](https://github.com/bsamseth/Goldfish/actions/workflows/cargo-build-test.yml)
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
-   [X] Tapered evaluation
-   [ ] Aspiration window search
-   [ ] Late move reductions
-   [ ] Static exchange evaluation for move ordering
-   [ ] More quiet move ordering heuristics
-   [ ] ProbCut
-   etc.

Each significant change will result in a new version of the engine (see
[releases](https://github.com/bsamseth/Goldfish/releases)). In the following
you see a relative rating between the current versions, based on test matches
played with a variety of (short) time controls. In this rating system, `v1.13`
is held at its [CCRL 40/40](http://computerchess.org.uk/ccrl/4040/) rating, and
the others are adjusted accordingly. Additionally, Stockfish with its
adjustable `SkillLevel` is included for reference. This gives an impression of
the relative improvements of the engine over time, but cannot be compared
directly to any other rating systems (e.g. FIDE).


 |   # | PLAYER               |   RATING  | POINTS  | PLAYED   | (%)|
 |:---:|---|:---:|:---:|:---:|:---:|
 |  1 | Stockfish 14.1 (S7)    |  2378.2  |   46.0  |   78 | 59.0% |
 |  2 | Goldfish v2.1.0        |  2314.1  |  196.5  |  374 | 52.5% |
 |  3 | Stockfish 14.1 (S6)    |  2215.4  |  139.5  |  228 | 61.2% |
 |  4 | Stockfish 14.1 (S5)    |  2172.3  |  139.5  |  247 | 56.5% |
 |  5 | Goldfish v2.0.0        |  2104.2  | 1114.0  | 1945 | 57.3% |
 |  6 | Goldfish v2.0.0-rc7    |  2030.4  |  687.0  | 1332 | 51.6% |
 |  7 | Goldfish v1.13.0       |  1983.0  | 1348.0  | 2622 | 51.4% |
 |  8 | Stockfish 14.1 (S4)    |  1974.2  |   63.5  |  214 | 29.7% |
 |  9 | Goldfish v1.12.1       |  1932.0  |  748.0  | 1613 | 46.4% |
 | 10 | Goldfish v1.12.0       |  1906.3  |  833.5  | 1528 | 54.5% |
 | 11 | Goldfish v1.11.1       |  1893.0  | 1095.0  | 2015 | 54.3% |
 | 12 | Goldfish v1.9.0        |  1878.3  | 1826.5  | 3538 | 51.6% |
 | 13 | Goldfish v1.11.0       |  1878.0  |  549.5  | 1100 | 50.0% |
 | 14 | Goldfish v2.0.0-rc6    |  1861.2  |  136.0  |  338 | 40.2% |
 | 15 | Goldfish v2.0.0-rc3    |  1853.6  |  177.5  |  417 | 42.6% |
 | 16 | Goldfish v2.0.0-rc5    |  1848.7  |  120.5  |  316 | 38.1% |
 | 17 | Goldfish v2.0.0-rc1    |  1835.2  |  217.5  |  436 | 49.9% |
 | 18 | Goldfish v2.0.0-rc2    |  1833.9  |  141.0  |  354 | 39.8% |
 | 19 | Goldfish v1.7.0        |  1813.7  | 1086.5  | 2053 | 52.9% |
 | 20 | Goldfish v1.8.2        |  1808.8  |  392.0  |  783 | 50.1% |
 | 21 | Goldfish v1.7.1        |  1806.6  |  244.0  |  477 | 51.2% |
 | 22 | Goldfish v1.8.0        |  1799.2  |  324.0  |  650 | 49.8% |
 | 23 | Goldfish v2.0.0-rc4    |  1798.8  |   19.5  |   57 | 34.2% |
 | 24 | Goldfish v1.8.1        |  1796.2  |  485.5  | 1000 | 48.5% |
 | 25 | Goldfish v1.6.0        |  1783.7  |  625.0  | 1151 | 54.3% |
 | 26 | Goldfish v1.7.2        |  1780.8  |   69.5  |  150 | 46.3% |
 | 27 | Goldfish v1.5.1        |  1726.0  |  645.5  | 1435 | 45.0% |
 | 28 | Goldfish v1.5          |  1723.7  |  554.5  | 1145 | 48.4% |
 | 29 | Goldfish v1.4          |  1720.9  |  646.5  | 1325 | 48.8% |
 | 30 | Goldfish v1.3          |  1706.2  |  314.0  |  680 | 46.2% |
 | 31 | Goldfish v1.2          |  1675.7  |  237.5  |  585 | 40.6% |
 | 32 | Goldfish v1.1          |  1634.5  |  210.0  |  597 | 35.2% |
 | 33 | Goldfish v1.0          |  1629.2  |  129.5  |  397 | 32.6% |


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
