#pragma once

#include <array>

namespace goldfish {

enum class CastlingType {
    KING_SIDE, QUEEN_SIDE, NO_CASTLING_TYPE
};

namespace CastlingTypes {

constexpr int VALUES_SIZE = 2;
constexpr std::array<CastlingType, VALUES_SIZE> values = {
    CastlingType::KING_SIDE, CastlingType::QUEEN_SIDE
};

}
}

