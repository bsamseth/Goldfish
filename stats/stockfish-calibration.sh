#!/bin/bash

cutechess-cli \
	-engine \
	conf='Goldfish v2.0.0-rc8' \
	-engine \
	conf='Stockfish 14.1 (S6)' \
	-engine \
	conf='Stockfish 14.1 (S5)' \
	-engine \
	conf='Stockfish 14.1 (S4)' \
	-each \
	tc=40/60 \
	proto=uci \
	option.Hash=512 \
	option.SyzygyPath=/home/bendik/goldfish/syzygy \
	book=/home/bendik/goldfish/openingbooks/gm2001.bin \
	bookdepth=6 \
	-games 2 \
	-rounds 500 \
	-pgnout /home/bendik/goldfish/stats/match-history.pgn min "fi" \
	-concurrency 10 \
	-ratinginterval 5 
