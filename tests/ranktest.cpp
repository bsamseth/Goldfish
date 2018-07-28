#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(ranktest, test_values) {
    for (auto rank : Ranks::values) {
        EXPECT_EQ(rank, Ranks::values[rank]);
    }
}

TEST(ranktest, test_inversion) {
    EXPECT_EQ(Rank::R8, Ranks::invert(Rank::R1));
    EXPECT_EQ(Rank::R7, Ranks::invert(Rank::R2));
    EXPECT_EQ(Rank::R6, Ranks::invert(Rank::R3));
    EXPECT_EQ(Rank::R5, Ranks::invert(Rank::R4));
    EXPECT_EQ(Rank::R4, Ranks::invert(Rank::R5));
    EXPECT_EQ(Rank::R3, Ranks::invert(Rank::R6));
    EXPECT_EQ(Rank::R2, Ranks::invert(Rank::R7));
    EXPECT_EQ(Rank::R1, Ranks::invert(Rank::R8));
}
