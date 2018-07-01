/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(ranktest, test_values
) {
for (
auto rank
: Rank::values) {
EXPECT_EQ(rank, Rank::values[rank]
);
}
}
