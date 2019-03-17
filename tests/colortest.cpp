#include "color.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(colortest, test_values)
{
    for (auto color : Colors::values)
    {
        EXPECT_EQ(color, Colors::values[color]);
    }
}

TEST(colortest, test_opposite)
{
    EXPECT_EQ(Color::WHITE, ~Color::BLACK);
    EXPECT_EQ(Color::BLACK, ~Color::WHITE);
}
