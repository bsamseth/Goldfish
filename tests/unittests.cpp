// Bring in my package's API, which is what I'm testing
#include "types.h"
// Bring in gtest
#include <gtest/gtest.h>

// Declare a test
TEST(TestSuite, testCase1) {
  ASSERT_EQ(1, 1);
}


// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
