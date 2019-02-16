#include "evaluation.hpp"

namespace goldfish::Evaluation {

constexpr int MAX_WEIGHT = 100;
constexpr int material_weight = 100;
constexpr int mobility_weight = 80;
constexpr int pawn_weight = 80;

Value evaluate_material(Color color, const Position &position);
Value evaluate_mobility(Color color, const Position &position);
Value evaluate_mobility(const Position &position, Square square, const std::vector<Direction>& directions);
Value evaluate_pawns(Color color, const Position& position);


Value evaluate(const Position &position) {
    // Initialize
    Color my_color = position.active_color;
    Color opposite_color = ~my_color;
    Value value = Value::ZERO;

    // Evaluate material
    value += (evaluate_material(my_color, position) - evaluate_material(opposite_color, position))
             * material_weight / MAX_WEIGHT;

    // Evaluate mobility
    value += (evaluate_mobility(my_color, position) - evaluate_mobility(opposite_color, position))
             * mobility_weight / MAX_WEIGHT;

    // Evaluate pawns
    value += (evaluate_pawns(my_color, position) - evaluate_pawns(opposite_color, position))
             * pawn_weight / MAX_WEIGHT;

    // Add Tempo
    value += Value::TEMPO;

    return value;
}

Value evaluate_material(Color color, const Position& position) {
    Value material = position.material[color];

    // Add bonus for bishop pair
    if (Bitboard::size(position.pieces[color][PieceType::BISHOP]) >= 2) {
        material += 50;
    }

    return material;
}

Value evaluate_mobility(Color color, const Position& position) {
    int knight_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::KNIGHT];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = Square(Bitboard::next(squares));
        knight_mobility += evaluate_mobility(position, square, Squares::knight_directions);
    }

    int bishop_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::BISHOP];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = Square(Bitboard::next(squares));
        bishop_mobility += evaluate_mobility(position, square, Squares::bishop_directions);
    }

    int rook_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::ROOK];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = Square(Bitboard::next(squares));
        rook_mobility += evaluate_mobility(position, square, Squares::rook_directions);
    }

    int queen_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::QUEEN];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = Square(Bitboard::next(squares));
        queen_mobility += evaluate_mobility(position, square, Squares::queen_directions);
    }

    return Value(knight_mobility * 4
               + bishop_mobility * 5
               + rook_mobility * 2
               + queen_mobility);
}

Value evaluate_mobility(const Position &position, Square square, const std::vector<Direction> &directions) {
    int mobility = 0;
    bool sliding = PieceTypes::is_sliding(Pieces::get_type(position.board[square]));

    for (auto direction : directions) {
        Square target_square = square + direction;

        while (Squares::is_valid(target_square)) {
            mobility++;

            if (sliding && position.board[target_square] == Piece::NO_PIECE) {
                target_square += direction;
            } else {
                break;
            }
        }
    }

    return Value(mobility);
}

Value evaluate_pawns(Color color, const Position& position) {
    const Direction forward    = color == Color::WHITE ? Direction::NORTH : Direction::SOUTH;
    const Direction right      = color == Color::WHITE ? Direction::EAST : Direction::WEST;
    const Direction left       = color == Color::WHITE ? Direction::WEST : Direction::EAST;
    const Direction backward   = color == Color::WHITE ? Direction::SOUTH : Direction::NORTH;
    const U64 our_pawns        = position.pieces[color][PieceType::PAWN];
    const U64 their_pawns      = position.pieces[~color][PieceType::PAWN];

    Value v = Value::ZERO;

    for (U64 squares = our_pawns; squares != 0; squares = Bitboard::remainder(squares)) {
        const U64 pawn_bb = 1ULL << Bitboard::number_of_trailing_zeros(squares);
        const Square pawn_sq = Square(Bitboard::next(squares));
        const Rank pawn_rank = Squares::get_rank(pawn_sq);
        const U64 pawn_fileBB = Squares::file_bb(pawn_sq);
        const U64 pawn_rankBB = Squares::rank_bb(pawn_sq);
        const U64 right_file = Squares::file_bb(pawn_sq + right);
        const U64 left_file = Squares::file_bb(pawn_sq + left);
        const U64 two_in_front_rank = Squares::rank_bb(pawn_sq + forward + forward);
        const U64 behind_rank = Squares::rank_bb(pawn_sq + backward);
        const U64 ranks_in_front = color == Color::WHITE ?
                Ranks::range(pawn_rank + Rank::RANK_1) : Ranks::range(Rank::RANK_1, pawn_rank - Rank::RANK_1);

        const bool passed = (their_pawns & ranks_in_front & (pawn_fileBB | left_file | right_file)) == 0;

        if (passed)
            v += 15;
        // Connected?
        if ((right_file | left_file) & our_pawns & (behind_rank | pawn_rankBB))
            v += passed ? 50 : 5;
        // Isolated?
        if (!((right_file | left_file) & our_pawns))
            v -= 10;
        // Backward?
        else if ((right_file | left_file) & their_pawns & two_in_front_rank)
            v -= 10;
        // Doubled? Value counted for all pawns on the file.
        if ((pawn_fileBB & our_pawns) != pawn_bb)
            v -= 10;
    }

    return v;
}

}
