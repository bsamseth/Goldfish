#pragma once

#include "movelist.hpp"
#include "position.hpp"

namespace goldfish
{
class MoveGenerator
{
public:
    MoveList<MoveEntry>&
        get_legal_moves(const Position& position, int depth, bool is_check);

    MoveList<MoveEntry>& get_moves(const Position& position, int depth, bool is_check);

private:
    MoveList<MoveEntry> moves;

    void add_moves(MoveList<MoveEntry>& list, const Position& position);

    void add_moves(MoveList<MoveEntry>&          list,
                   Square                        origin_square,
                   const std::vector<Direction>& directions,
                   const Position&               position);

    void add_pawn_moves(MoveList<MoveEntry>& list,
                        Square               pawn_square,
                        const Position&      position);

    void add_castling_moves(MoveList<MoveEntry>& list,
                            Square               king_square,
                            const Position&      position);
};

}  // namespace goldfish
