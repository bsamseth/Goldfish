#include "movelist.hpp"

#include <algorithm>
#include <cassert>

namespace goldfish
{
/**
 * Sorts the move list using a stable insertion sort.
 */
template <class T>
void MoveList<T>::sort(int first_n_sorted)
{
    for (int i = first_n_sorted; i < size; i++)
    {
        const T entry = std::move(entries[i]);

        int j = i;
        while ((j > 0) && (entries[j - 1].value < entry.value))
        {
            entries[j] = entries[j - 1];
            j--;
        }

        entries[j] = std::move(entry);
    }
}

/**
 * Move the given move to the front of the array, keeping
 * all others in the same order.
 */
template <class T>
void MoveList<T>::add_killer(Move m)
{
    for (auto killer = entries.rbegin(); killer != entries.rend(); ++killer)
    {
        if ((*killer).move == m)
        {
            // Right shift the subarray [begin, entry] by one shift.
            std::rotate(killer, killer + 1, entries.rend());
            return;
        }
    }
    // The move should always be in the list.
    assert(false);
}

/**
 * Rates the moves in the list according to "Most Valuable Victim - Least Valuable
 * Aggressor".
 */
template <class T>
void MoveList<T>::rate_from_Mvvlva()
{
    for (int i = 0; i < size; i++)
    {
        Move  move  = entries[i].move;
        Value value = Value::ZERO;

        Value piece_type_value
            = PieceTypes::get_value(Pieces::get_type(Moves::get_origin_piece(move)));
        value += Value::KING_VALUE / piece_type_value;

        Piece target = Moves::get_target_piece(move);
        if (Pieces::is_valid(target))
        {
            value += 10 * PieceTypes::get_value(Pieces::get_type(target));
        }

        entries[i].value = value;
    }
}

template class MoveList<MoveEntry>;

template class MoveList<RootEntry>;

}  // namespace goldfish
