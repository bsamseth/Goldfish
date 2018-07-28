#include "file.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(filetest, test_values ) {
    for (auto file : Files::values) {
        EXPECT_EQ(file, Files::values[file]);
    }
}

TEST(filetest, test_inversion) {
    EXPECT_EQ(File::A, Files::invert(File::H));
    EXPECT_EQ(File::B, Files::invert(File::G));
    EXPECT_EQ(File::C, Files::invert(File::F));
    EXPECT_EQ(File::D, Files::invert(File::E));
    EXPECT_EQ(File::E, Files::invert(File::D));
    EXPECT_EQ(File::F, Files::invert(File::C));
    EXPECT_EQ(File::G, Files::invert(File::B));
    EXPECT_EQ(File::H, Files::invert(File::A));
}

