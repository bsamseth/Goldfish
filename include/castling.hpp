#pragma once

#include <array>
#include "color.hpp"
#include "castlingtype.hpp"

namespace goldfish {

enum Castling {
    NO_CASTLING,
    WHITE_KING_SIDE,
    WHITE_QUEEN_SIDE = WHITE_KING_SIDE << 1,
    BLACK_KING_SIDE = WHITE_KING_SIDE << 2,
    BLACK_QUEEN_SIDE = WHITE_KING_SIDE << 3,

    WHITE_CASTLING = WHITE_KING_SIDE | WHITE_QUEEN_SIDE,
    BLACK_CASTLING = BLACK_KING_SIDE | BLACK_QUEEN_SIDE,
    ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

    CASTLING_RIGHT_NB = 16
};

constexpr Castling operator|(Color c, CastlingType s) {
  return Castling(Castling::WHITE_KING_SIDE << ((s == CastlingType::QUEEN_SIDE) + 2 * c));
}

constexpr Castling operator|(Castling a, Castling b) { return Castling(((int)a) | ((int) b)); }
constexpr Castling operator&(Castling a, Castling b) { return Castling(((int)a) & ((int) b)); }
constexpr Castling& operator|=(Castling& a, Castling b) { return a = a | b; }
constexpr Castling& operator|=(Castling& a, int b) { return a = Castling(a | b); }
constexpr Castling& operator&=(Castling& a, Castling b) { return a = a & b; }
constexpr Castling& operator&=(Castling& a, int b) { return a = Castling(a & b); }

namespace Castlings {

constexpr int VALUES_LENGTH = 16;

constexpr CastlingType type_of(Castling c) {
    return c & (Castling::WHITE_KING_SIDE | Castling::BLACK_KING_SIDE) ?
            CastlingType::KING_SIDE : CastlingType::QUEEN_SIDE;
}

constexpr Color color_of(Castling c) {
    return c & Castling::WHITE_CASTLING ? Color::WHITE : Color::BLACK;
}

}
}
