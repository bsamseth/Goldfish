#include "square.hpp"

#include "file.hpp"
#include "rank.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(squaretest, test_values)
{
    for (auto rank : Ranks::values)
    {
        for (auto file : Files::values)
        {
            Square square = Squares::value_of(file, rank);

            EXPECT_EQ(file, Squares::get_file(square));
            EXPECT_EQ(rank, Squares::get_rank(square));
        }
    }
}
