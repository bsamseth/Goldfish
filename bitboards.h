#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <string>

#include "types.h"

const Bitboard BITBOARD_EMPTY = 0;
const Bitboard BITBOARD_UNIVERSE = -1;

const Bitboard RANK_1_BB = 0xff;
const Bitboard RANK_2_BB = 0xff00;
const Bitboard RANK_7_BB = 0xff000000000000;
const Bitboard RANK_8_BB = (BITBOARD_UNIVERSE ^ (RANK_1_BB << (7*8)));


std::string Bitboards::prettyString(Bitboard b);

#endif
