#pragma once

#include <array>

namespace goldfish
{
enum CastlingType
{
    KING_SIDE = 0,
    QUEEN_SIDE,
    NO_CASTLING_TYPE
};

namespace CastlingTypes
{
constexpr int                                   VALUES_SIZE = 2;
constexpr std::array<CastlingType, VALUES_SIZE> values      = {KING_SIDE, QUEEN_SIDE};

}  // namespace CastlingTypes
}  // namespace goldfish
