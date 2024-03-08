#!/bin/bash

cutechess-cli \
	-engine \
	cmd=/home/bendik/goldfish/src/target/release-lto/goldfish \
	name=alpha \
	-engine \
	cmd=goldfish-1.5.1 \
	-each \
	tc=40/60 \
	proto=uci \
	option.Hash=500 \
	book=/home/bendik/goldfish/openingbooks/gm2001.bin \
	bookdepth=6 \
	-rounds 100 \
	-pgnout /home/bendik/goldfish/stats/baseline.pgn \
	-concurrency 10
