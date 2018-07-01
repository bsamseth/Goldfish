/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "color.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(colortest, test_values
) {
for (
auto color
: Color::values) {
EXPECT_EQ(color, Color::values[color]
);
}
}

TEST(colortest, test_opposite
) {
EXPECT_EQ(+Color::WHITE,
Color::swap_color(Color::BLACK)
);
EXPECT_EQ(+Color::BLACK,
Color::swap_color(Color::WHITE)
);
}
