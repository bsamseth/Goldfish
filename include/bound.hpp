#pragma once

#include <cstdint>

namespace goldfish {

enum class Bound : uint8_t {
    NONE, LOWER, UPPER, EXACT = LOWER | UPPER
};

}
