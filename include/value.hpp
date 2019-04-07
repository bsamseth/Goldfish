#pragma once

#include "depth.hpp"
#include "operations.hpp"

#include <cmath>
#include <cstdlib>

namespace goldfish
{
enum Value : int
{
    ZERO                = 0,
    DRAW                = 0,
    CHECKMATE           = 30000,
    CHECKMATE_THRESHOLD = CHECKMATE - Depth::MAX_PLY,
    INFINITE            = 2 * CHECKMATE,
    NO_VALUE            = 3 * CHECKMATE,
    KNOWN_WIN           = CHECKMATE_THRESHOLD - 1,
    KNOWN_LOSS          = -KNOWN_WIN,

    // Piece values as defined by Larry Kaufman
    PAWN_VALUE   = 100,
    KNIGHT_VALUE = 325,
    BISHOP_VALUE = 325,
    ROOK_VALUE   = 500,
    QUEEN_VALUE  = 975,
    KING_VALUE   = 20000,

    TEMPO = 1,

    RAZOR_MARGIN    = 600,
    FUTILITY_MARGIN = 200,
};

ENABLE_FULL_OPERATORS_ON(Value)

constexpr Value operator+(Value v, int i)
{
    return Value(int(v) + i);
}
constexpr Value operator-(Value v, int i)
{
    return Value(int(v) - i);
}
inline Value& operator+=(Value& v, int i)
{
    return v = v + i;
}
inline Value& operator-=(Value& v, int i)
{
    return v = v - i;
}

namespace Values
{
constexpr bool is_checkmate(Value value)
{
    Value absvalue = Value(value < 0 ? -value : value);
    return absvalue >= Value::CHECKMATE_THRESHOLD && absvalue <= Value::CHECKMATE;
}

constexpr bool is_checkmate_in(Value value, Depth depth)
{
    Value absvalue = Value(value < 0 ? -value : value);
    return absvalue + depth >= Value::CHECKMATE;
}
}  // namespace Values
}  // namespace goldfish
