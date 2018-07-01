/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "castlingtype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(castlingtypetest, test_values
) {
for (
auto castlingtype
: CastlingType::values) {
EXPECT_EQ(castlingtype, CastlingType::values[castlingtype]
);
}
}
