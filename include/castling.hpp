#pragma once

#include "castlingtype.hpp"
#include "color.hpp"

#include <array>

namespace goldfish
{
enum Castling
{
    WHITE_KING_SIDE  = 1 << 0,
    WHITE_QUEEN_SIDE = 1 << 1,
    BLACK_KING_SIDE  = 1 << 2,
    BLACK_QUEEN_SIDE = 1 << 3,

    NO_CASTLING = 0,
};

constexpr Castling operator|(Castling a, Castling b)
{
    return Castling(((int) a) | ((int) b));
}
constexpr Castling operator&(Castling a, Castling b)
{
    return Castling(((int) a) & ((int) b));
}
inline Castling& operator|=(Castling& a, Castling b)
{
    return a = a | b;
}
inline Castling& operator|=(Castling& a, int b)
{
    return a = Castling(a | b);
}
inline Castling& operator&=(Castling& a, Castling b)
{
    return a = a & b;
}
inline Castling& operator&=(Castling& a, int b)
{
    return a = Castling(a & b);
}

namespace Castlings
{
constexpr int VALUES_LENGTH = 16;

inline constexpr Castling value_of(Color color, CastlingType castlingtype)
{
    switch (color)
    {
        case Color::WHITE:
            switch (castlingtype)
            {
                case CastlingType::KING_SIDE: return WHITE_KING_SIDE;
                case CastlingType::QUEEN_SIDE: return WHITE_QUEEN_SIDE;
                default: throw std::invalid_argument("Bad castlingtype");
            }
        case Color::BLACK:
            switch (castlingtype)
            {
                case CastlingType::KING_SIDE: return BLACK_KING_SIDE;
                case CastlingType::QUEEN_SIDE: return BLACK_QUEEN_SIDE;
                default: throw std::invalid_argument("Bad castlingtype");
            }
        default: throw std::invalid_argument("Bad color");
    }
}

inline constexpr CastlingType get_type(Castling castling)
{
    switch (castling)
    {
        case WHITE_KING_SIDE:
        case BLACK_KING_SIDE: return CastlingType::KING_SIDE;
        case WHITE_QUEEN_SIDE:
        case BLACK_QUEEN_SIDE: return CastlingType::QUEEN_SIDE;
        default: throw std::invalid_argument("Bad castling");
    }
}

inline constexpr Color get_color(Castling castling)
{
    switch (castling)
    {
        case WHITE_KING_SIDE:
        case WHITE_QUEEN_SIDE: return Color::WHITE;
        case BLACK_KING_SIDE:
        case BLACK_QUEEN_SIDE: return Color::BLACK;
        default: throw std::invalid_argument("Bad castling");
    }
}
}  // namespace Castlings
}  // namespace goldfish
