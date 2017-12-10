[![Build Status](https://travis-ci.org/bsamseth/Goldfish.svg?branch=master)](https://travis-ci.org/bsamseth/Goldfish)
[![Coverage Status](https://coveralls.io/repos/github/bsamseth/Goldfish/badge.svg?branch=master)](https://coveralls.io/github/bsamseth/Goldfish?branch=master)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/bsamseth/Goldfish/blob/master/LICENCE)
[![Project Status: WIP – Initial development is in progress, but there has not yet been a stable, usable release suitable for the public.](http://www.repostatus.org/badges/latest/wip.svg)](http://www.repostatus.org/#wip)

# Goldfish
###### Stockfish's very distant and not so bright cousin

This is a UCI chess engine. The projects current end vision is to be able to play and hopefully lose against the Goldfish engine in a GUI. 

This is meant as a project to work on as practice/just for the fun of it. Contributions are welcome if you feel like it.

## Build

The project is built using CMake, which hopefully makes this as portable as possible. Recommend building in a separate directory:

``` bash
$ mkdir build && cd build
$ cmake ..
$ make
```

After the compiling is done you should have two executables in the build directory: `goldfish` and `unit_tests`. The former is the interface to the engine it self.

## Run

After building the `goldfish` executable, you _can_ run it directly. The engine speaks using the UCI protocol. An example from the conversation between Scid and the engine:

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

Although it is possible to use the text based interface directly, it's recommended to run this through a UCI compatible graphical user interface, such as Scid.

## Strength

At the moment, Goldfish just tries to suggest a _legal_ move, choosing at random. It's also quite unstable.
