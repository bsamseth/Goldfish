#include <notation.hpp>
#include "value.hpp"
#include "prettyprint.hpp"

namespace goldfish::Values {

std::array<std::array<Score, Squares::VALUES_LENGTH>, Pieces::VALUES_SIZE> psqt;

void init_psqt() {
    for (Piece piece : Pieces::values) {
        PieceType piece_type = Pieces::get_type(piece);
        Color color = Pieces::get_color(piece);

        for (Square sq : Squares::values) {
            Rank rank = Squares::get_rank(sq);
            File file = Squares::get_file(sq);
            if (color == Color::BLACK)
                rank = Ranks::invert(rank);
            if (file > File::D)
                file = Files::invert(file);

            psqt[piece][sq] = Score(Values::get_value(piece_type)) + piece_square_bonus[piece_type][rank][file];
        }
    }
}


}
