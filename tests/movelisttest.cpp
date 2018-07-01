/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "movelist.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(movelisttest, test
) {
MoveList <MoveEntry> move_list;

EXPECT_EQ(0, move_list.size);

move_list.entries[move_list.size++]->
move = 1;
EXPECT_EQ(1, move_list.size);
}
