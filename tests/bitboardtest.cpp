/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "bitboard.hpp"
#include "square.hpp"

#include "gtest/gtest.h"

#include <random>
#include <list>
#include <algorithm>

using namespace goldfish;

class BitboardTest : public ::testing::Test {
protected:
    std::list<int> pool;

    virtual void SetUp() {
        std::default_random_engine generator;

        while (pool.size() < 64) {
            std::uniform_int_distribution<int> distribution(0, 63);
            int value = distribution(generator);
            if (std::find(pool.begin(), pool.end(), Square::values[value]) == pool.end()) {
                pool.push_back(Square::values[value]);
            }
        }
    }
};

TEST_F(BitboardTest, should_add_allSquaresCorrectly
) {
uint64_t bitboard = 0;

for (
auto x88square
: pool) {
bitboard = Bitboard::add(x88square, bitboard);
}

EXPECT_EQ(bitboard, std::numeric_limits<uint64_t>::max()
);
}

TEST_F(BitboardTest, should_remove_allSquaresCorrectly
) {
uint64_t bitboard = std::numeric_limits<uint64_t>::max();

for (
auto x88square
: pool) {
bitboard = Bitboard::remove(x88square, bitboard);
}

EXPECT_EQ(bitboard,
0);
}

TEST(bitboardtest, should_return_theNextSquare
) {
uint64_t bitboard = Bitboard::add(Square::A6, 0);

int square = Bitboard::next(bitboard);

EXPECT_EQ(square,
+Square::A6);
}

TEST(bitboardtest, should_return_correctRemainder
) {
uint64_t bitboard = 0b1110100;

uint64_t remainder = Bitboard::remainder(bitboard);

EXPECT_EQ(remainder,
0b1110000);
}

TEST(bitboardtest, should_return_correctSize
) {
uint64_t bitboard = 0b111;

int size = Bitboard::size(bitboard);

EXPECT_EQ(size,
3);
}

TEST(bitboardtest, test_number_ofTrailingZeros
) {
uint64_t bitboard = 0;
int i = 0;

for (
auto square
: Square::values) {
bitboard = Bitboard::add(square, bitboard);

EXPECT_EQ(i, Bitboard::number_of_trailing_zeros(bitboard)
);

bitboard = Bitboard::remove(square, bitboard);
i++;
}
}

TEST(bitboardtest, test_bit_count
) {
std::default_random_engine generator;

for (
int i = 0;
i < 1000; i++) {
uint64_t bitboard = 0;
int count = 0;

int index = 0;
while (true) {
std::uniform_int_distribution<int> distribution(1, 4);
index +=
distribution(generator);
if (index < 64) {
bitboard |= 1ULL <<
index;
count++;
} else {
break;
}
}

EXPECT_EQ(count, Bitboard::bit_count(bitboard)
);
}
}