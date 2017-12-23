#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "evaluate.h"

using namespace std;

TEST (Evaluate, equalBlackAndWhite) {
    EXPECT_EQ(piece_value(W_KNIGHT, SQ_B3), -piece_value(B_KNIGHT, SQ_B6));
    EXPECT_EQ(piece_value(W_PAWN, SQ_A3), -piece_value(B_PAWN, SQ_A6));
    EXPECT_EQ(piece_value(W_PAWN, SQ_A6), -piece_value(B_PAWN, SQ_A3));
}
