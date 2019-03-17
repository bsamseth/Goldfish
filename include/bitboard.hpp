#pragma once

#include <array>
#include <cstdint>

namespace goldfish
{
using U64 = uint64_t;

namespace Bitboard
{
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
    return number_of_trailing_zeros(bitboard);
}

constexpr U64 remainder(U64 bitboard)
{
    return bitboard & (bitboard - 1);
}

constexpr int size(U64 bitboard)
{
    return bit_count(bitboard);
}

}  // namespace Bitboard
}  // namespace goldfish
