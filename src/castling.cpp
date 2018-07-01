#include <stdexcept>
#include "castling.hpp"
#include "color.hpp"
#include "castlingtype.hpp"

namespace goldfish {

int Castling::value_of(int color, int castlingtype) {
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

int Castling::get_type(int castling) {
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

int Castling::get_color(int castling) {
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
