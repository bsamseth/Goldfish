#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "move.h"
#include "types.h"
#include "stateinfo.h"

using namespace std;

TEST (StateInfo, init) {
  StateInfo s = {}, s2 = {};
  StateInfo s3 = {};
  EXPECT_EQ(1, s2.id-s.id);
  EXPECT_EQ(2, s3.id-s.id);
  EXPECT_EQ(NO_PIECE, s2.lastMove_destinationPiece);
  EXPECT_EQ(NO_SQUARE, s.lastMove_enpassantTarget);
  EXPECT_EQ(nullptr, s3.previous);
}
