#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <string>

#include "types.h"

const Bitboard BITBOARD_EMPTY = 0;
const Bitboard BITBOARD_UNIVERSE = -1;

const Bitboard RANK_1_BB = 0xff;
const Bitboard RANK_2_BB = RANK_1_BB << 8; // 0xff00;
const Bitboard RANK_7_BB = RANK_1_BB << 8*6;
const Bitboard RANK_8_BB = RANK_1_BB << 8*7; // test me! TODO


std::string Bitboards::prettyString(Bitboard b);

#endif
