/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "piecetype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(piecetypetest, test_values
) {
for (
auto piecetype
: PieceType::values) {
EXPECT_EQ(piecetype, PieceType::values[piecetype]
);
}
}

TEST(piecetypetest, test_is_validPromotion
) {
EXPECT_TRUE (PieceType::is_valid_promotion(PieceType::KNIGHT));

EXPECT_TRUE (PieceType::is_valid_promotion(PieceType::BISHOP));

EXPECT_TRUE (PieceType::is_valid_promotion(PieceType::ROOK));

EXPECT_TRUE (PieceType::is_valid_promotion(PieceType::QUEEN));

EXPECT_FALSE (PieceType::is_valid_promotion(PieceType::PAWN));

EXPECT_FALSE (PieceType::is_valid_promotion(PieceType::KING));

EXPECT_FALSE (PieceType::is_valid_promotion(PieceType::NO_PIECE_TYPE));

}

TEST(piecetypetest, test_is_sliding
) {
EXPECT_TRUE (PieceType::is_sliding(PieceType::BISHOP));

EXPECT_TRUE (PieceType::is_sliding(PieceType::ROOK));

EXPECT_TRUE (PieceType::is_sliding(PieceType::QUEEN));

EXPECT_FALSE (PieceType::is_sliding(PieceType::PAWN));

EXPECT_FALSE (PieceType::is_sliding(PieceType::KNIGHT));

EXPECT_FALSE (PieceType::is_sliding(PieceType::KING));

}
