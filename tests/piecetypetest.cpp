#include "piecetype.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(piecetypetest, test_values)
{
    for (auto piecetype : PieceTypes::values)
    {
        EXPECT_EQ(piecetype, PieceTypes::values[piecetype]);
    }
}

TEST(piecetypetest, test_is_validPromotion)
{
    EXPECT_TRUE(PieceTypes::is_valid_promotion(PieceType::KNIGHT));
    EXPECT_TRUE(PieceTypes::is_valid_promotion(PieceType::BISHOP));
    EXPECT_TRUE(PieceTypes::is_valid_promotion(PieceType::ROOK));
    EXPECT_TRUE(PieceTypes::is_valid_promotion(PieceType::QUEEN));
    EXPECT_FALSE(PieceTypes::is_valid_promotion(PieceType::PAWN));
    EXPECT_FALSE(PieceTypes::is_valid_promotion(PieceType::KING));
    EXPECT_FALSE(PieceTypes::is_valid_promotion(PieceType::NO_PIECE_TYPE));
}

TEST(piecetypetest, test_is_sliding)
{
    EXPECT_TRUE(PieceTypes::is_sliding(PieceType::BISHOP));
    EXPECT_TRUE(PieceTypes::is_sliding(PieceType::ROOK));
    EXPECT_TRUE(PieceTypes::is_sliding(PieceType::QUEEN));
    EXPECT_FALSE(PieceTypes::is_sliding(PieceType::PAWN));
    EXPECT_FALSE(PieceTypes::is_sliding(PieceType::KNIGHT));
    EXPECT_FALSE(PieceTypes::is_sliding(PieceType::KING));
}
