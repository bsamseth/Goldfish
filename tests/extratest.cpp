// Bring in my package's API, which is what I'm testing
#include "types.h"
// Bring in gtest
#include <gtest/gtest.h>

// Declare a test
TEST(TestSuite, testCase2) {
  ASSERT_EQ(1, 2);
}

