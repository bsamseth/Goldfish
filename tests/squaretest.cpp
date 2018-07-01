/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "square.hpp"
#include "file.hpp"
#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(squaretest, test_values
) {
for (
auto rank
: Rank::values) {
for (
auto file
: File::values) {
int square = Square::value_of(file, rank);

EXPECT_EQ(file, Square::get_file(square)
);
EXPECT_EQ(rank, Square::get_rank(square)
);
}
}
}
