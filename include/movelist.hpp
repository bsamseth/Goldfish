#pragma once

#include "move.hpp"
#include "value.hpp"

#include <array>

namespace goldfish
{
/**
 * This class stores our moves for a specific position. For the root node we
 * will populate pv for every root move.
 */
template <class T>
class MoveList
{
private:
    static const int MAX_MOVES = 256;

public:
    std::array<T, MAX_MOVES> entries;
    int                      size = 0;

    MoveList() = default;

    void sort(int first_n_sorted = 1);

    void rate_from_Mvvlva();

    void sort_as_best(Move m);

    bool add_killer(Move m);
};

class MoveVariation
{
public:
    std::array<Move, Depth::MAX_PLY> moves;
    int                              size = 0;
};

class MoveEntry
{
public:
    Move  move  = Move::NO_MOVE;
    Value value = Value::NO_VALUE;
};

class RootEntry : public MoveEntry
{
public:
    MoveVariation pv;
};

}  // namespace goldfish
