#include "movelist.hpp"

namespace goldfish {

template<class T>
MoveList<T>::MoveList() {
    for (unsigned int i = 0; i < entries.size(); i++) {
        entries[i] = std::shared_ptr<T>(new T());
    }
}

/**
 * Sorts the move list using a stable insertion sort.
 */
template<class T>
void MoveList<T>::sort() {
    for (int i = 1; i < size; i++) {
        std::shared_ptr<T> entry(entries[i]);

        int j = i;
        while ((j > 0) && (entries[j - 1]->value < entry->value)) {
            entries[j] = entries[j - 1];
            j--;
        }

        entries[j] = entry;
    }
}

/**
 * Rates the moves in the list according to "Most Valuable Victim - Least Valuable Aggressor".
 */
template<class T>
void MoveList<T>::rate_from_Mvvlva() {
    for (int i = 0; i < size; i++) {
        Move move = entries[i]->move;
        Value value = Value::ZERO;

        Value piece_type_value = PieceTypes::get_value(Pieces::get_type(Moves::get_origin_piece(move)));
        value += Value::KING_VALUE / piece_type_value;

        Piece target = Moves::get_target_piece(move);
        if (Pieces::is_valid(target)) {
            value += 10 * PieceTypes::get_value(Pieces::get_type(target));
        }

        entries[i]->value = value;
    }
}

template
class MoveList<MoveEntry>;

template
class MoveList<RootEntry>;

}
