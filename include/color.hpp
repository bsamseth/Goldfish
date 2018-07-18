#pragma once

#include <array>
#include <cassert>

#include "operations.hpp"

namespace goldfish {

enum Color {
    WHITE, BLACK, NO_COLOR
};

inline constexpr Color operator~(Color c) {
    assert(c != Color::NO_COLOR);
    return c == Color::WHITE ? Color::BLACK : Color::WHITE;
}


namespace Colors {

constexpr int VALUES_SIZE = 2;

constexpr std::array<Color, VALUES_SIZE> values = {
    Color::WHITE, Color::BLACK
};

}

}

