/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "file.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(filetest, test_values
) {
for (
auto file
: File::values) {
EXPECT_EQ(file, File::values[file]
);
}
}
