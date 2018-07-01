#include <stdexcept>
#include "color.hpp"

namespace goldfish {

const std::array<int, Color::VALUES_SIZE> Color::values = {WHITE, BLACK};

int Color::swap_color(int color) {
    switch (color) {
        case WHITE:
            return BLACK;
        case BLACK:
            return WHITE;
        default:
            throw std::invalid_argument("Bad color value.");
    }
}

}
