#pragma once

#include "position.hpp"
#include "movelist.hpp"

namespace goldfish {

class MoveGenerator {
public:
    MoveList<MoveEntry> &get_legal_moves(Position &position, int depth, bool is_check);

    MoveList<MoveEntry> &get_moves(Position &position, int depth, bool is_check);

private:
    MoveList<MoveEntry> moves;

    void add_moves(MoveList<MoveEntry> &list, Position &position);

    void
    add_moves(MoveList<MoveEntry> &list, int origin_square, const std::vector<int> &directions, Position &position);

    void add_pawn_moves(MoveList<MoveEntry> &list, int pawn_square, Position &position);

    void add_castling_moves(MoveList<MoveEntry> &list, int king_square, Position &position);
};

}
