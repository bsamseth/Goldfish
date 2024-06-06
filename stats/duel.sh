#!/bin/bash

cutechess-cli \
	-engine \
	conf='Goldfish v2.0.0-rc6' \
	-engine \
	conf='Goldfish v2.0.0-rc3' \
	-engine \
	conf='Goldfish v2.0.0-rc5' \
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
	-rounds 40 \
	-pgnout /home/bendik/goldfish/stats/match-history.pgn min fi \
	-concurrency 10 \
	-tournament gauntlet