#include "file.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(filetest, test_values ) {
    for (auto file : Files::values) {
        EXPECT_EQ(file, Files::values[static_cast<int>(file)]);
    }
}
