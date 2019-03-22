#pragma once

#include "color.hpp"
#include "square.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace goldfish
{
using U64 = uint64_t;

namespace Bitboard
{
const std::string pretty(U64 b);

constexpr U64 AllSquares  = ~U64(0);
constexpr U64 DarkSquares = 0xAA55AA55AA55AA55ULL;

constexpr U64 FileABB = 0x0101010101010101ULL;
constexpr U64 FileBBB = FileABB << 1;
constexpr U64 FileCBB = FileABB << 2;
constexpr U64 FileDBB = FileABB << 3;
constexpr U64 FileEBB = FileABB << 4;
constexpr U64 FileFBB = FileABB << 5;
constexpr U64 FileGBB = FileABB << 6;
constexpr U64 FileHBB = FileABB << 7;

constexpr U64 Rank1BB = 0xFF;
constexpr U64 Rank2BB = Rank1BB << (8 * 1);
constexpr U64 Rank3BB = Rank1BB << (8 * 2);
constexpr U64 Rank4BB = Rank1BB << (8 * 3);
constexpr U64 Rank5BB = Rank1BB << (8 * 4);
constexpr U64 Rank6BB = Rank1BB << (8 * 5);
constexpr U64 Rank7BB = Rank1BB << (8 * 6);
constexpr U64 Rank8BB = Rank1BB << (8 * 7);

constexpr U64 DEBRUIJN64 = 0x03F79D71B4CB0A89ULL;

constexpr std::array<int, 64> lsb_table
    = {0,  47, 1,  56, 48, 27, 2,  60, 57, 49, 41, 37, 28, 16, 3,  61,
       54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4,  62,
       46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
       25, 39, 14, 33, 19, 30, 9,  24, 13, 18, 8,  12, 7,  6,  5,  63};

constexpr int bit_count(U64 b)
{
    b = b - ((b >> 1) & 0x5555555555555555ULL);
    b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);
    b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (b * 0x0101010101010101ULL) >> 56;
}

constexpr int number_of_trailing_zeros(U64 b)
{
    return lsb_table[((b ^ (b - 1)) * DEBRUIJN64) >> 58];
}

constexpr int to_bit_square(int square)
{
    return ((square & ~7) >> 1) | (square & 7);
}

constexpr int to_x88_square(int square)
{
    return ((square & ~7) << 1) | (square & 7);
}

constexpr U64 add(int square, U64 bitboard)
{
    return bitboard | 1ULL << to_bit_square(square);
}

constexpr U64 remove(int square, U64 bitboard)
{
    return bitboard & ~(1ULL << to_bit_square(square));
}

constexpr int next(U64 bitboard)
{
    return to_x88_square(number_of_trailing_zeros(bitboard));
}

constexpr U64 remainder(U64 bitboard)
{
    return bitboard & (bitboard - 1);
}

constexpr int size(U64 bitboard)
{
    return bit_count(bitboard);
}

template <Direction D>
constexpr U64 shift(U64 b)
{
    return D == NORTH
               ? b << 8
               : D == SOUTH
                     ? b >> 8
                     : D == EAST
                           ? (b & ~FileHBB) << 1
                           : D == WEST ? (b & ~FileABB) >> 1
                                       : D == NORTH_EAST
                                             ? (b & ~FileHBB) << 9
                                             : D == NORTH_WEST
                                                   ? (b & ~FileABB) << 7
                                                   : D == SOUTH_EAST
                                                         ? (b & ~FileHBB) >> 7
                                                         : D == SOUTH_WEST
                                                               ? (b & ~FileABB) >> 9
                                                               : 0;
}
//
/// pawn_attacks_bb() returns the squares attacked by pawns of the given color
/// from the squares in the given bitboard.
template <Color C>
constexpr U64 pawn_attacks_bb(U64 b)
{
    return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

}  // namespace Bitboard
}  // namespace goldfish
