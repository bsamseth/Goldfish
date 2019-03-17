#pragma once

#include <array>
#include "operations.hpp"

namespace goldfish {

enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};


ENABLE_INCR_OPERATORS_ON(File)

constexpr File operator~(File f) {
    return File(f ^ FILE_H); // Horizontal flip FILE_A -> FILE_H
}

namespace Files {

constexpr int VALUES_SIZE = 8;
constexpr std::array<File, VALUES_SIZE> values = {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

constexpr bool is_valid(File file) {
    return FILE_A <= file && file <= FILE_H;
}

}
}

