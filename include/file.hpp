#pragma once

#include <array>

#include "operations.hpp"

namespace goldfish {

enum File {
    A, B, C, D, E, F, G, H, NO_FILE
};

ENABLE_BASE_OPERATORS_ON(File);
ENABLE_INCR_OPERATORS_ON(File);

namespace Files {

constexpr int VALUES_SIZE = 8;

constexpr std::array<File, VALUES_SIZE> values = {
    File::A, File::B, File::C, File::D, File::E, File::F, File::G, File::H
};

inline constexpr bool is_valid(File file) {
    return file != File::NO_FILE;
}

}

}

