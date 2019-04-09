#pragma once

#include "color.hpp"
#include "piecetype.hpp"

#include <array>

namespace goldfish
{
enum Piece
{
    WHITE_PAWN = 0,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    NO_PIECE,
};

namespace Pieces
{
constexpr int VALUES_SIZE = 12;

constexpr std::array<Piece, VALUES_SIZE> values = {WHITE_PAWN,
                                                   WHITE_KNIGHT,
                                                   WHITE_BISHOP,
                                                   WHITE_ROOK,
                                                   WHITE_QUEEN,
                                                   WHITE_KING,
                                                   BLACK_PAWN,
                                                   BLACK_KNIGHT,
                                                   BLACK_BISHOP,
                                                   BLACK_ROOK,
                                                   BLACK_QUEEN,
                                                   BLACK_KING};

constexpr int MASK = 0x1F;

constexpr bool is_valid(Piece piece)
{
    return piece != Piece::NO_PIECE;
}

constexpr Piece value_of(Color color, PieceType piecetype)
{
    switch (color)
    {
        case Color::WHITE:
            switch (piecetype)
            {
                case PieceType::PAWN: return WHITE_PAWN;
                case PieceType::KNIGHT: return WHITE_KNIGHT;
                case PieceType::BISHOP: return WHITE_BISHOP;
                case PieceType::ROOK: return WHITE_ROOK;
                case PieceType::QUEEN: return WHITE_QUEEN;
                case PieceType::KING: return WHITE_KING;
                default: throw std::invalid_argument("Bad piecetype");
            }
        case Color::BLACK:
            switch (piecetype)
            {
                case PieceType::PAWN: return BLACK_PAWN;
                case PieceType::KNIGHT: return BLACK_KNIGHT;
                case PieceType::BISHOP: return BLACK_BISHOP;
                case PieceType::ROOK: return BLACK_ROOK;
                case PieceType::QUEEN: return BLACK_QUEEN;
                case PieceType::KING: return BLACK_KING;
                default: throw std::invalid_argument("Bad piecetype");
            }
        default: throw std::invalid_argument("Bad color");
    }
}

inline constexpr PieceType get_type(Piece piece)
{
    switch (piece)
    {
        case WHITE_PAWN:
        case BLACK_PAWN: return PieceType::PAWN;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT: return PieceType::KNIGHT;
        case WHITE_BISHOP:
        case BLACK_BISHOP: return PieceType::BISHOP;
        case WHITE_ROOK:
        case BLACK_ROOK: return PieceType::ROOK;
        case WHITE_QUEEN:
        case BLACK_QUEEN: return PieceType::QUEEN;
        case WHITE_KING:
        case BLACK_KING: return PieceType::KING;
        default: throw std::invalid_argument("Bad piece");
    }
}

inline constexpr Color get_color(Piece piece)
{
    switch (piece)
    {
        case WHITE_PAWN:
        case WHITE_KNIGHT:
        case WHITE_BISHOP:
        case WHITE_ROOK:
        case WHITE_QUEEN:
        case WHITE_KING: return Color::WHITE;
        case BLACK_PAWN:
        case BLACK_KNIGHT:
        case BLACK_BISHOP:
        case BLACK_ROOK:
        case BLACK_QUEEN:
        case BLACK_KING: return Color::BLACK;
        default: throw std::invalid_argument("Bad piece");
    }
}

}  // namespace Pieces
}  // namespace goldfish
