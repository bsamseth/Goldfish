#include "castling.hpp"

#include "castlingtype.hpp"
#include "color.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(castlingtest, test_value_of)
{
    EXPECT_EQ(Castling::WHITE_KING_SIDE,
              Castlings::value_of(Color::WHITE, CastlingType::KING_SIDE));
    EXPECT_EQ(Castling::WHITE_QUEEN_SIDE,
              Castlings::value_of(Color::WHITE, CastlingType::QUEEN_SIDE));
    EXPECT_EQ(Castling::BLACK_KING_SIDE,
              Castlings::value_of(Color::BLACK, CastlingType::KING_SIDE));
    EXPECT_EQ(Castling::BLACK_QUEEN_SIDE,
              Castlings::value_of(Color::BLACK, CastlingType::QUEEN_SIDE));
}
