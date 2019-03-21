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

ENABLE_INCR_OPERATORS_ON(Color)

inline constexpr Color operator~(Color c)
{
    assert(c != Color::NO_COLOR);
    return c == Color::WHITE ? Color::BLACK : Color::WHITE;
}

namespace Colors
{
constexpr int                            VALUES_SIZE = 2;
constexpr std::array<Color, VALUES_SIZE> values      = {WHITE, BLACK};

}  // namespace Colors
}  // namespace goldfish
