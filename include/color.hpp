#pragma once

#include <array>

namespace goldfish {

class Color {
public:
    static const int WHITE = 0;
    static const int BLACK = 1;

    static const int NO_COLOR = 2;

    static const int VALUES_SIZE = 2;
    static const std::array<int, VALUES_SIZE> values;

    static int swap_color(int color);

    Color() = delete;

    ~Color() = delete;
};

}

