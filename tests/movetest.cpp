/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "move.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(movetest, test_creation
) {
int move = Move::value_of(MoveType::PAWN_PROMOTION, Square::A7, Square::B8, Piece::WHITE_PAWN, Piece::BLACK_QUEEN,
                          PieceType::KNIGHT);

EXPECT_EQ(+MoveType::PAWN_PROMOTION,
Move::get_type(move)
);
EXPECT_EQ(+Square::A7,
Move::get_origin_square(move)
);
EXPECT_EQ(+Square::B8,
Move::get_target_square(move)
);
EXPECT_EQ(+Piece::WHITE_PAWN,
Move::get_origin_piece(move)
);
EXPECT_EQ(+Piece::BLACK_QUEEN,
Move::get_target_piece(move)
);
EXPECT_EQ(+PieceType::KNIGHT,
Move::get_promotion(move)
);
}

TEST(movetest, test_promotion
) {
int move = Move::value_of(MoveType::PAWN_PROMOTION, Square::B7, Square::C8, Piece::WHITE_PAWN, Piece::BLACK_QUEEN,
                          PieceType::KNIGHT);

EXPECT_EQ(+PieceType::KNIGHT,
Move::get_promotion(move)
);
}
