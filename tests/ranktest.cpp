#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(ranktest, test_values)
{
    for (auto rank : Ranks::values)
    {
        EXPECT_EQ(rank, Ranks::values[rank]);
    }
}
