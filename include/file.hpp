#pragma once

#include <array>

namespace goldfish {

class File {
public:

    enum {
        A, B, C, D, E, F, G, H, NO_FILE
    };

    static const int VALUES_SIZE = 8;
    static const std::array<int, VALUES_SIZE> values;

    static bool is_valid(int file);

    File() = delete;

    ~File() = delete;
};

}

