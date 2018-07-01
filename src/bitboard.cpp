#include "bitboard.hpp"
#include "square.hpp"

namespace goldfish {

const std::array<int, 64> Bitboard::lsb_table = {
        0, 47, 1, 56, 48, 27, 2, 60,
        57, 49, 41, 37, 28, 16, 3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11, 4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30, 9, 24,
        13, 18, 8, 12, 7, 6, 5, 63
};

uint64_t Bitboard::add(int square, uint64_t bitboard) {
    return bitboard | 1ULL << to_bit_square(square);
}

uint64_t Bitboard::remove(int square, uint64_t bitboard) {
    return bitboard & ~(1ULL << to_bit_square(square));
}

int Bitboard::next(uint64_t bitboard) {
    return to_x88_square(number_of_trailing_zeros(bitboard));
}

uint64_t Bitboard::remainder(uint64_t bitboard) {
    return bitboard & (bitboard - 1);
}

int Bitboard::size(uint64_t bitboard) {
    return bit_count(bitboard);
}

int Bitboard::to_x88_square(int square) {
    return ((square & ~7) << 1) | (square & 7);
}

int Bitboard::to_bit_square(int square) {
    return ((square & ~7) >> 1) | (square & 7);
}

int Bitboard::number_of_trailing_zeros(uint64_t b) {
    return lsb_table[((b ^ (b - 1)) * DEBRUIJN64) >> 58];
}

int Bitboard::bit_count(uint64_t b) {
    b = b - ((b >> 1) & 0x5555555555555555ULL);
    b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);
    b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (b * 0x0101010101010101ULL) >> 56;
}

}