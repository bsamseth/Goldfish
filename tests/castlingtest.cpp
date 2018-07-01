/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "color.hpp"
#include "castlingtype.hpp"
#include "castling.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(castlingtest, test_value_of
) {
EXPECT_EQ(+Castling::WHITE_KING_SIDE,
Castling::value_of(Color::WHITE, CastlingType::KING_SIDE
));
EXPECT_EQ(+Castling::WHITE_QUEEN_SIDE,
Castling::value_of(Color::WHITE, CastlingType::QUEEN_SIDE
));
EXPECT_EQ(+Castling::BLACK_KING_SIDE,
Castling::value_of(Color::BLACK, CastlingType::KING_SIDE
));
EXPECT_EQ(+Castling::BLACK_QUEEN_SIDE,
Castling::value_of(Color::BLACK, CastlingType::QUEEN_SIDE
));
}
