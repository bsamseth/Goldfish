#include "evaluation.hpp"

namespace goldfish::Evaluation {

int evaluate(Position &position) {
    // Initialize
    Color my_color = position.active_color;
    Color opposite_color = ~my_color;
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

int evaluate_material(Color color, Position &position) {
    int material = position.material[static_cast<int>(color)];

    // Add bonus for bishop pair
    if (Bitboard::size(position.pieces[static_cast<int>(color)][static_cast<int>(PieceType::BISHOP)]) >= 2) {
        material += 50;
    }

    return material;
}

int evaluate_mobility(Color color, Position &position) {
    int knight_mobility = 0;
    for (auto squares = position.pieces[static_cast<int>(color)][static_cast<int>(PieceType::KNIGHT)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        knight_mobility += evaluate_mobility(color, position, square, Squares::knight_directions);
    }

    int bishop_mobility = 0;
    for (auto squares = position.pieces[static_cast<int>(color)][static_cast<int>(PieceType::BISHOP)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        bishop_mobility += evaluate_mobility(color, position, square, Squares::bishop_directions);
    }

    int rook_mobility = 0;
    for (auto squares = position.pieces[static_cast<int>(color)][static_cast<int>(PieceType::ROOK)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        rook_mobility += evaluate_mobility(color, position, square, Squares::rook_directions);
    }

    int queen_mobility = 0;
    for (auto squares = position.pieces[static_cast<int>(color)][static_cast<int>(PieceType::QUEEN)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        int square = Bitboard::next(squares);
        queen_mobility += evaluate_mobility(color, position, square, Squares::queen_directions);
    }

    return knight_mobility * 4
           + bishop_mobility * 5
           + rook_mobility * 2
           + queen_mobility;
}

int evaluate_mobility(Color color, Position &position, int square, const std::vector<int> &directions) {
    int mobility = 0;
    bool sliding = PieceTypes::is_sliding(Pieces::get_type(position.board[square]));

    for (auto direction : directions) {
        Square target_square = static_cast<Square>(static_cast<int>(square) + direction);

        while (Squares::is_valid(target_square)) {
            mobility++;

            if (sliding && position.board[static_cast<int>(target_square)] == Piece::NO_PIECE) {
                target_square = static_cast<Square>(static_cast<int>(target_square) + direction);
            } else {
                break;
            }
        }
    }

    return mobility;
}

}
