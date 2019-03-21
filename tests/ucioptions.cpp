#include "tt.hpp"
#include "uci.hpp"

#include "gtest/gtest.h"
#include <string>

using namespace goldfish;

TEST(ucioptions, init_and_update)
{
    UCI::init(UCI::Options);

    // Check for default value and correct init.
    ASSERT_EQ(16U, (size_t) UCI::Options["Hash"]);
    ASSERT_EQ(16U, TT.size());

    // Updating the Hash should trigger the resize of the table.
    UCI::Options["Hash"] = std::string {"10"};
    ASSERT_EQ(10U, TT.size());
}
