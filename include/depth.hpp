#pragma once

#include <cstdint>

namespace goldfish {

using Depth = uint16_t;

namespace Depths {

constexpr Depth MAX_PLY = 256;
constexpr Depth MAX_DEPTH = 64;

}
}
