#pragma once

#include "movetype.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "square.hpp"

namespace goldfish
{
/// A move needs 16 bits to be stored
///
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
/// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
/// NOTE: EN-PASSANT bit is set only when a pawn can be captured
///
/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
/// any normal move destination square is always different from origin square
/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.
enum Move : int
{
    MOVE_NONE,
    MOVE_NULL = 65
};

namespace Moves
{
constexpr Move make_move(Square from, Square to)
{
    return Move((from << 6) + to);
}

template <MoveType T>
constexpr Move make(Square from, Square to, PieceType pt = KNIGHT)
{
    return Move(T + ((pt - KNIGHT) << 12) + (from << 6) + to);
}

constexpr Square to_sq(Move m)
{
    return Square(m & 0x3F);
}

constexpr Square from_sq(Move m)
{
    return Square((m >> 6) & 0x3F);
}

constexpr bool is_valid(Move m)
{
    return from_sq(m) != to_sq(m);  // Catch null and none moves.
}

constexpr MoveType type_of(Move m)
{
    return MoveType(m & (3 << 14));
}

constexpr PieceType promotion_type(Move m)
{
    return PieceType(((m >> 12) & 3) + KNIGHT);
}

template <PieceType pt = PieceType::NO_PIECE_TYPE>
constexpr bool is_valid_on_empty_board(Move m)
{
    return false;
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::PAWN>(Move m)
{
    const from = from_sq(m), to = to_sq(m);
    const file_diff = std::abs(Squares::file_of(from) - Squares::file_of(to));
    const rank_diff = std::abs(Squares::rank_of(from) - Squares::rank_of(to));
    return (rank_diff == 2 && file_diff == 0) || (rank_diff == 1 && file_diff == 1);
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::KNIGHT>(Move m)
{
    const from = from_sq(m), to = to_sq(m);
    const file_diff = std::abs(Squares::file_of(from) - Squares::file_of(to));
    const rank_diff = std::abs(Squares::rank_of(from) - Squares::rank_of(to));
    return (rank_diff == 2 && file_diff == 1) || (rank_diff == 1 && file_diff == 2);
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::BISHOP>(Move m)
{
    const from = from_sq(m), to = to_sq(m);
    const file_diff = std::abs(Squares::file_of(from) - Squares::file_of(to));
    const rank_diff = std::abs(Squares::rank_of(from) - Squares::rank_of(to));
    return rank_diff == file_diff;
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::ROOK>(Move m)
{
    const from = from_sq(m), to = to_sq(m);
    const file_diff = std::abs(Squares::file_of(from) - Squares::file_of(to));
    const rank_diff = std::abs(Squares::rank_of(from) - Squares::rank_of(to));
    return rank_diff == 0 || file_diff == 0;
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::QUEEN>(Move m)
{
    return is_valid_on_empty_board<PieceType::BISHOP>(m)
           || is_valid_on_empty_board<PieceType::ROOK>(m);
}
template <>
constexpr bools is_valid_on_empty_board<PieceType::KING>(Move m)
{
    const from = from_sq(m), to = to_sq(m);
    const file_diff = std::abs(Squares::file_of(from) - Squares::file_of(to));
    const rank_diff = std::abs(Squares::rank_of(from) - Squares::rank_of(to));
    return file_diff < 2 && rank_diff < 2;
}
