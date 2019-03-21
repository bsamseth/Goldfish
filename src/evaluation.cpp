#include "evaluation.hpp"

namespace goldfish::Evaluation
{
Value evaluate(const Position& position)
{
    // Initialize
    Color my_color       = position.active_color;
    Color opposite_color = ~my_color;
    Value value          = Value::ZERO;

    // Evaluate material
    Value material_score = (evaluate_material(my_color, position)
                            - evaluate_material(opposite_color, position))
                           * material_weight / MAX_WEIGHT;
    value += material_score;

    // Evaluate mobility
    Value mobility_score = (evaluate_mobility(my_color, position)
                            - evaluate_mobility(opposite_color, position))
                           * mobility_weight / MAX_WEIGHT;
    value += mobility_score;

    // Add Tempo
    value += Value::TEMPO;

    return value;
}

Value evaluate_material(Color color, const Position& position)
{
    Value material = position.material[color];

    // Add bonus for bishop pair
    if (Bitboard::size(position.pieces[color][PieceType::BISHOP]) >= 2)
    {
        material += 50;
    }

    return material;
}

Value evaluate_mobility(Color color, const Position& position)
{
    int knight_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::KNIGHT]; squares != 0;
         squares      = Bitboard::remainder(squares))
    {
        Square square = Square(Bitboard::next(squares));
        knight_mobility
            += evaluate_mobility(position, square, Squares::knight_directions);
    }

    int bishop_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::BISHOP]; squares != 0;
         squares      = Bitboard::remainder(squares))
    {
        Square square = Square(Bitboard::next(squares));
        bishop_mobility
            += evaluate_mobility(position, square, Squares::bishop_directions);
    }

    int rook_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::ROOK]; squares != 0;
         squares      = Bitboard::remainder(squares))
    {
        Square square = Square(Bitboard::next(squares));
        rook_mobility += evaluate_mobility(position, square, Squares::rook_directions);
    }

    int queen_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::QUEEN]; squares != 0;
         squares      = Bitboard::remainder(squares))
    {
        Square square = Square(Bitboard::next(squares));
        queen_mobility
            += evaluate_mobility(position, square, Squares::queen_directions);
    }

    return Value(knight_mobility * 4 + bishop_mobility * 5 + rook_mobility * 2
                 + queen_mobility);
}

Value evaluate_mobility(const Position&               position,
                        Square                        square,
                        const std::vector<Direction>& directions)
{
    int  mobility = 0;
    bool sliding  = PieceTypes::is_sliding(Pieces::get_type(position.board[square]));

    for (auto direction : directions)
    {
        Square target_square = square + direction;

        while (Squares::is_valid(target_square))
        {
            mobility++;

            if (sliding && position.board[target_square] == Piece::NO_PIECE)
            {
                target_square += direction;
            }
            else
            {
                break;
            }
        }
    }

    return Value(mobility);
}

}  // namespace goldfish::Evaluation
