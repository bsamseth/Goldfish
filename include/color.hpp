#pragma once

#include "operations.hpp"

#include <array>
#include <cassert>

namespace goldfish
{
enum Color
{
    WHITE = 0,
    BLACK,
    NO_COLOR
};

ENABLE_INCR_OPERATORS_ON(Color);

constexpr Color operator~(Color c)
{
    return Color(c ^ BLACK);
}

namespace Colors
{
constexpr int                            VALUES_SIZE = 2;
constexpr std::array<Color, VALUES_SIZE> values      = {WHITE, BLACK};

}  // namespace Colors
}  // namespace goldfish
