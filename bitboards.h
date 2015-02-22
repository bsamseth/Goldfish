#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <string>

#include "types.h"

const Bitboard BITBOARD_EMPTY = 0;
const Bitboard BITBOARD_UNIVERSE = -1;

std::string Bitboards::prettyString(Bitboard b);

#endif
