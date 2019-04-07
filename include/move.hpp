#pragma once

#include "movetype.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "square.hpp"

namespace goldfish
{
namespace Moves
{
constexpr int TYPE_SHIFT          = 0;
constexpr int TYPE_MASK           = MoveTypes::MASK << TYPE_SHIFT;
constexpr int ORIGIN_SQUARE_SHIFT = 3;
constexpr int ORIGIN_SQUARE_MASK  = Squares::MASK << ORIGIN_SQUARE_SHIFT;
constexpr int TARGET_SQUARE_SHIFT = 10;
constexpr int TARGET_SQUARE_MASK  = Squares::MASK << TARGET_SQUARE_SHIFT;
constexpr int ORIGIN_PIECE_SHIFT  = 17;
constexpr int ORIGIN_PIECE_MASK   = Pieces::MASK << ORIGIN_PIECE_SHIFT;
constexpr int TARGET_PIECE_SHIFT  = 22;
constexpr int TARGET_PIECE_MASK   = Pieces::MASK << TARGET_PIECE_SHIFT;
constexpr int PROMOTION_SHIFT     = 27;
constexpr int PROMOTION_MASK      = PieceTypes::MASK << PROMOTION_SHIFT;

}  // namespace Moves

/**
 * This class represents a move as a int value. The fields are represented by
 * the following bits.
 * <p/>
 * <code> 0 -  2</code>: type (required)
 * <code> 3 -  9</code>: origin square (required)
 * <code>10 - 16</code>: target square (required)
 * <code>17 - 21</code>: origin piece (required)
 * <code>22 - 26</code>: target piece (optional)
 * <code>27 - 29</code>: promotion type (optional)
 */
enum Move : int
{
    NO_MOVE = (MoveType::NO_MOVE_TYPE << Moves::TYPE_SHIFT)
              | (Square::NO_SQUARE << Moves::ORIGIN_SQUARE_SHIFT)
              | (Square::NO_SQUARE << Moves::TARGET_SQUARE_SHIFT)
              | (Piece::NO_PIECE << Moves::ORIGIN_PIECE_SHIFT)
              | (Piece::NO_PIECE << Moves::TARGET_PIECE_SHIFT)
              | (PieceType::NO_PIECE_TYPE << Moves::PROMOTION_SHIFT)
};

namespace Moves
{

constexpr bool is_valid(Move move)
{
    return move != Move::NO_MOVE;
}

constexpr Move value_of(MoveType  type,
                        Square    origin_square,
                        Square    target_square,
                        Piece     origin_piece,
                        Piece     target_piece,
                        PieceType promotion)
{
    int move = 0;
    move |= type << TYPE_SHIFT;
    move |= origin_square << ORIGIN_SQUARE_SHIFT;
    move |= target_square << TARGET_SQUARE_SHIFT;
    move |= origin_piece << ORIGIN_PIECE_SHIFT;
    move |= target_piece << TARGET_PIECE_SHIFT;
    move |= promotion << PROMOTION_SHIFT;
    return Move(move);
}

constexpr MoveType get_type(Move move)
{
    return MoveType((move & TYPE_MASK) >> TYPE_SHIFT);
}

constexpr Square get_origin_square(Move move)
{
    return Square((move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT);
}

constexpr Square get_target_square(Move move)
{
    return Square((move & TARGET_SQUARE_MASK) >> TARGET_SQUARE_SHIFT);
}

constexpr Piece get_origin_piece(Move move)
{
    return Piece((move & ORIGIN_PIECE_MASK) >> ORIGIN_PIECE_SHIFT);
}

constexpr Piece get_target_piece(Move move)
{
    return Piece((move & TARGET_PIECE_MASK) >> TARGET_PIECE_SHIFT);
}

constexpr PieceType get_promotion(Move move)
{
    return PieceType((move & PROMOTION_MASK) >> PROMOTION_SHIFT);
}

constexpr bool is_capture(Move m)
{
    return get_target_piece(m) != Piece::NO_PIECE;
}
}  // namespace Moves
}  // namespace goldfish
