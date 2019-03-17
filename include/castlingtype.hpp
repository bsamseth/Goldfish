#pragma once

#include <array>

namespace goldfish
{
enum CastlingType
{
    KING_SIDE,
    QUEEN_SIDE,
    CASTLING_SIDE_NB = 2
};

namespace CastlingTypes
{
constexpr int                                   VALUES_SIZE = 2;
constexpr std::array<CastlingType, VALUES_SIZE> values      = {KING_SIDE, QUEEN_SIDE};

}  // namespace CastlingTypes
}  // namespace goldfish
