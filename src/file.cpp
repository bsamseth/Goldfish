#include "file.hpp"

namespace goldfish {

const std::array<int, File::VALUES_SIZE> File::values = {
        A, B, C, D, E, F, G, H
};

bool File::is_valid(int file) {
    switch (file) {
        case A:
        case B:
        case C:
        case D:
        case E:
        case F:
        case G:
        case H:
            return true;
        default:
            return false;
    }
}

}
