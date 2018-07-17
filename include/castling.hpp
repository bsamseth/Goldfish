#pragma once

#include <array>
#include <stdexcept>

#include "castlingtype.hpp"


namespace goldfish::Castling {

constexpr int WHITE_KING_SIDE  = 1 << 0;
constexpr int WHITE_QUEEN_SIDE = 1 << 1;
constexpr int BLACK_KING_SIDE  = 1 << 2;
constexpr int BLACK_QUEEN_SIDE = 1 << 3;

constexpr int NO_CASTLING = 0;

constexpr int VALUES_LENGTH = 16;

inline constexpr int value_of(Color color, CastlingType castlingtype) {
    switch (color) {
        case Color::WHITE:
            switch (castlingtype) {
                case CastlingType::KING_SIDE:
                    return WHITE_KING_SIDE;
                case CastlingType::QUEEN_SIDE:
                    return WHITE_QUEEN_SIDE;
                default:
                    throw std::invalid_argument("Bad castlingtype");
            }
        case Color::BLACK:
            switch (castlingtype) {
                case CastlingType::KING_SIDE:
                    return BLACK_KING_SIDE;
                case CastlingType::QUEEN_SIDE:
                    return BLACK_QUEEN_SIDE;
                default:
                    throw std::invalid_argument("Bad castlingtype");
            }
        default:
            throw std::invalid_argument("Bad color");
    }
}

inline constexpr CastlingType get_type(int castling) {
    switch (castling) {
        case WHITE_KING_SIDE:
        case BLACK_KING_SIDE:
            return CastlingType::KING_SIDE;
        case WHITE_QUEEN_SIDE:
        case BLACK_QUEEN_SIDE:
            return CastlingType::QUEEN_SIDE;
        default:
            throw std::invalid_argument("Bad castling");
    }
}

inline constexpr Color get_color(int castling) {
    switch (castling) {
        case WHITE_KING_SIDE:
        case WHITE_QUEEN_SIDE:
            return Color::WHITE;
        case BLACK_KING_SIDE:
        case BLACK_QUEEN_SIDE:
            return Color::BLACK;
        default:
            throw std::invalid_argument("Bad castling");
    }
}
}
