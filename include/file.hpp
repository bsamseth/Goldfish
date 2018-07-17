#pragma once

#include <array>

namespace goldfish {

enum class File {
    A, B, C, D, E, F, G, H, NO_FILE
};

inline constexpr File& operator++ (File& file) {
    return file = static_cast<File>(static_cast<int>(file) + 1);
}

inline constexpr File& operator+= (File& file, int increment) {
    int file_n = static_cast<int>(file) + increment;

    if (file_n >= static_cast<int>(File::NO_FILE))
        return file = File::NO_FILE;

    return file = static_cast<File>(file_n);
}

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

