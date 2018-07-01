#include "evaluation.hpp"

namespace goldfish {

int Evaluation::material_weight = 100;
int Evaluation::mobility_weight = 80;

int Evaluation::evaluate(Position &position) {
    // Initialize
    int my_color = position.active_color;
    int opposite_color = Color::swap_color(my_color);
    int value = 0;

    // Evaluate material
    int material_score = (evaluate_material(my_color, position) - evaluate_material(opposite_color, position))
                         * material_weight / MAX_WEIGHT;
    value += material_score;

    // Evaluate mobility
    int mobility_score = (evaluate_mobility(my_color, position) - evaluate_mobility(opposite_color, position))
                         * mobility_weight / MAX_WEIGHT;
    value += mobility_score;

    // Add Tempo
    value += TEMPO;

    return value;
}

int Evaluation::evaluate_material(int color, Position &position) {
    int material = position.material[color];

    // Add bonus for bishop pair
    if (Bitboard::size(position.pieces[color][PieceType::BISHOP]) >= 2) {
        material += 50;
    }

    return material;
}

int Evaluation::evaluate_mobility(int color, Position &position) {
    int knight_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::KNIGHT];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        knight_mobility += evaluate_mobility(color, position, square, Square::knight_directions);
    }

    int bishop_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::BISHOP];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        bishop_mobility += evaluate_mobility(color, position, square, Square::bishop_directions);
    }

    int rook_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::ROOK];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        rook_mobility += evaluate_mobility(color, position, square, Square::rook_directions);
    }

    int queen_mobility = 0;
    for (auto squares = position.pieces[color][PieceType::QUEEN];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        queen_mobility += evaluate_mobility(color, position, square, Square::queen_directions);
    }

    return knight_mobility * 4
           + bishop_mobility * 5
           + rook_mobility * 2
           + queen_mobility;
}

int Evaluation::evaluate_mobility(int color, Position &position, int square, const std::vector<int> &directions) {
    int mobility = 0;
    bool sliding = PieceType::is_sliding(Piece::get_type(position.board[square]));

    for (auto direction : directions) {
        int target_square = square + direction;

        while (Square::is_valid(target_square)) {
            mobility++;

            if (sliding && position.board[target_square] == Piece::NO_PIECE) {
                target_square += direction;
            } else {
                break;
            }
        }
    }

    return mobility;
}

}
