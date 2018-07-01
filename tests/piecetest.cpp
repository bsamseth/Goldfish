/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "piece.hpp"
#include "color.hpp"
#include "piecetype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(piecetest, test_values
) {
for (
auto piece
: Piece::values) {
EXPECT_EQ(piece, Piece::values[piece]
);
}
}

TEST(piecetest, test_value_of
) {
EXPECT_EQ(+Piece::WHITE_PAWN,
Piece::value_of(Color::WHITE, PieceType::PAWN
));
EXPECT_EQ(+Piece::WHITE_KNIGHT,
Piece::value_of(Color::WHITE, PieceType::KNIGHT
));
EXPECT_EQ(+Piece::WHITE_BISHOP,
Piece::value_of(Color::WHITE, PieceType::BISHOP
));
EXPECT_EQ(+Piece::WHITE_ROOK,
Piece::value_of(Color::WHITE, PieceType::ROOK
));
EXPECT_EQ(+Piece::WHITE_QUEEN,
Piece::value_of(Color::WHITE, PieceType::QUEEN
));
EXPECT_EQ(+Piece::WHITE_KING,
Piece::value_of(Color::WHITE, PieceType::KING
));
EXPECT_EQ(+Piece::BLACK_PAWN,
Piece::value_of(Color::BLACK, PieceType::PAWN
));
EXPECT_EQ(+Piece::BLACK_KNIGHT,
Piece::value_of(Color::BLACK, PieceType::KNIGHT
));
EXPECT_EQ(+Piece::BLACK_BISHOP,
Piece::value_of(Color::BLACK, PieceType::BISHOP
));
EXPECT_EQ(+Piece::BLACK_ROOK,
Piece::value_of(Color::BLACK, PieceType::ROOK
));
EXPECT_EQ(+Piece::BLACK_QUEEN,
Piece::value_of(Color::BLACK, PieceType::QUEEN
));
EXPECT_EQ(+Piece::BLACK_KING,
Piece::value_of(Color::BLACK, PieceType::KING
));
}

TEST(piecetest, test_get_type
) {
EXPECT_EQ(+PieceType::PAWN,
Piece::get_type(Piece::WHITE_PAWN)
);
EXPECT_EQ(+PieceType::PAWN,
Piece::get_type(Piece::BLACK_PAWN)
);
EXPECT_EQ(+PieceType::KNIGHT,
Piece::get_type(Piece::WHITE_KNIGHT)
);
EXPECT_EQ(+PieceType::KNIGHT,
Piece::get_type(Piece::BLACK_KNIGHT)
);
EXPECT_EQ(+PieceType::BISHOP,
Piece::get_type(Piece::WHITE_BISHOP)
);
EXPECT_EQ(+PieceType::BISHOP,
Piece::get_type(Piece::BLACK_BISHOP)
);
EXPECT_EQ(+PieceType::ROOK,
Piece::get_type(Piece::WHITE_ROOK)
);
EXPECT_EQ(+PieceType::ROOK,
Piece::get_type(Piece::BLACK_ROOK)
);
EXPECT_EQ(+PieceType::QUEEN,
Piece::get_type(Piece::WHITE_QUEEN)
);
EXPECT_EQ(+PieceType::QUEEN,
Piece::get_type(Piece::BLACK_QUEEN)
);
EXPECT_EQ(+PieceType::KING,
Piece::get_type(Piece::WHITE_KING)
);
EXPECT_EQ(+PieceType::KING,
Piece::get_type(Piece::BLACK_KING)
);
}

TEST(piecetest, test_get_color
) {
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_PAWN)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_PAWN)
);
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_KNIGHT)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_KNIGHT)
);
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_BISHOP)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_BISHOP)
);
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_ROOK)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_ROOK)
);
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_QUEEN)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_QUEEN)
);
EXPECT_EQ(+Color::WHITE,
Piece::get_color(Piece::WHITE_KING)
);
EXPECT_EQ(+Color::BLACK,
Piece::get_color(Piece::BLACK_KING)
);
}
