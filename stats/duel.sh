#!/bin/bash

cutechess-cli \
	-engine \
	conf='Goldfish v2.0.0-rc8' \
	-engine \
	conf='Goldfish v2.0.0-rc7' \
	-engine \
	conf='Goldfish v1.13.0' \
	-each \
	tc=40/60 \
	proto=uci \
	option.Hash=500 \
	option.SyzygyPath=/home/bendik/goldfish/syzygy \
	book=/home/bendik/goldfish/openingbooks/gm2001.bin \
	bookdepth=6 \
	-games 2 \
	-rounds 500 \
	-pgnout /home/bendik/goldfish/stats/match-history.pgn min "fi" \
	-concurrency 10 \
	-ratinginterval 5 \
	-tournament gauntlet \
