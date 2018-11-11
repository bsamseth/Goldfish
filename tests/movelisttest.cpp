#include "movelist.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(movelisttest, test) {
    MoveList <MoveEntry> move_list;

    EXPECT_EQ(0, move_list.size);
    move_list.entries[move_list.size++]->move = Move::NO_MOVE;
    EXPECT_EQ(1, move_list.size);
}
