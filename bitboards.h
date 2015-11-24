#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <string>

#include "types.h"

const Bitboard BITBOARD_EMPTY = 0;
const Bitboard BITBOARD_UNIVERSE = -1;

const Bitboard RANK_1_BB = 0xff;
const Bitboard RANK_2_BB = RANK_1_BB << 8; // 0xff00;
const Bitboard RANK_3_BB = RANK_1_BB << 8*2;
const Bitboard RANK_4_BB = RANK_1_BB << 8*3;
const Bitboard RANK_5_BB = RANK_1_BB << 8*4;
const Bitboard RANK_6_BB = RANK_1_BB << 8*5;
const Bitboard RANK_7_BB = RANK_1_BB << 8*6;
const Bitboard RANK_8_BB = RANK_1_BB << 8*7; // test me! TODO

const Bitboard FILE_A_BB = 0x101010101010101;
const Bitboard FILE_B_BB = FILE_A_BB << 1;
const Bitboard FILE_C_BB = FILE_A_BB << 2;
const Bitboard FILE_D_BB = FILE_A_BB << 3;
const Bitboard FILE_E_BB = FILE_A_BB << 4;
const Bitboard FILE_F_BB = FILE_A_BB << 5;
const Bitboard FILE_G_BB = FILE_A_BB << 6;
const Bitboard FILE_H_BB = FILE_A_BB << 7;

inline Bitboard getFile_BB(Square s) {
    return FILE_A_BB << (int(s) % 8);
}

inline Bitboard getRank_BB(Square s) { 
  return RANK_1_BB << 8 * ((int(s) / 8) % 8);
}

inline int file_diff(Square s1, Square s2) {
  return (int(s2) % 8) - (int(s1) % 8);
}

inline int rank_diff(Square s1, Square s2) {
  return (int(s2) / 8) % 8 - (int(s1) / 8) % 8;
}

std::string Bitboards::prettyString(Bitboard b);

#endif
