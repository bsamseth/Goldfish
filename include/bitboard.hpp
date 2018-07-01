#pragma once

#include <array>
#include <cstdint>

namespace goldfish {

class Bitboard {
public:
    static uint64_t add(int square, uint64_t bitboard);

    static uint64_t remove(int square, uint64_t bitboard);

    static int next(uint64_t bitboard);

    static uint64_t remainder(uint64_t bitboard);

    static int size(uint64_t bitboard);

    static int number_of_trailing_zeros(uint64_t b);

    static int bit_count(uint64_t b);

private:
    static const uint64_t DEBRUIJN64 = 0x03F79D71B4CB0A89ULL;
    static const std::array<int, 64> lsb_table;

    static int to_x88_square(int square);

    static int to_bit_square(int square);
};

}
