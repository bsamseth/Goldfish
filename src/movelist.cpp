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
        int move = entries[i]->move;
        int value = 0;

        int piece_type_value = PieceType::get_value(Piece::get_type(Move::get_origin_piece(move)));
        value += PieceType::KING_VALUE / piece_type_value;

        int target = Move::get_target_piece(move);
        if (Piece::is_valid(target)) {
            value += 10 * PieceType::get_value(Piece::get_type(target));
        }

        entries[i]->value = value;
    }
}

template
class MoveList<MoveEntry>;

template
class MoveList<RootEntry>;

}
