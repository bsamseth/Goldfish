#include "castlingtype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(castlingtypetest, test_values)
{
    for (auto castlingtype : CastlingTypes::values)
    {
        EXPECT_EQ(castlingtype, CastlingTypes::values[castlingtype]);
    }
}
