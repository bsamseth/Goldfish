#pragma once

#include "operations.hpp"

namespace goldfish
{
enum Depth : int
{
    DEPTH_ZERO = 0,
    DEPTH_MAX  = 64,
    MAX_PLY    = 256,
};

ENABLE_FULL_OPERATORS_ON(Depth)

constexpr Depth operator+(Depth v, int i)
{
    return Depth(int(v) + i);
}
constexpr Depth operator-(Depth v, int i)
{
    return Depth(int(v) - i);
}
inline Depth& operator+=(Depth& v, int i)
{
    return v = v + i;
}
inline Depth& operator-=(Depth& v, int i)
{
    return v = v - i;
}

}  // namespace goldfish
