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

template <class T>
bool MoveList<T>::add_killer(Move m)
{
    if (m == Move::NO_MOVE || entries[0].move == m)
        return false;

    for (auto it = entries.begin() + 1; it != entries.end(); ++it)
    {
        if (it->move == m)
        {
            it->value += 1;  // Break ties (quiets affected most), propagate forward in list.
            sort(std::distance(entries.begin(), it));
            return true;
        }
    }

    return false;
}

/**
 * Move the given move to the front of the array, keeping
 * all others in the same order.
 */
template<class T>
void MoveList<T>::sort_as_best(Move m)
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
    assert(false && "Best move must be in movelist!");
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

        Piece target = Moves::get_target_piece(move);
        if (Pieces::is_valid(target))
        {
            // If this is a capture, score according to how valuable the target is,
            // while giving preference to captures using a less valuable attacker.
            value += 10 * PieceTypes::get_value(Pieces::get_type(target)) + Value::KING_VALUE / PieceTypes::get_value(Pieces::get_type(Moves::get_origin_piece(move)));
        }

        entries[i].value = value;
    }
}

template class MoveList<MoveEntry>;

template class MoveList<RootEntry>;

}  // namespace goldfish
