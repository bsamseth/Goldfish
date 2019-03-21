#pragma once

#include "move.hpp"
#include "value.hpp"

#include <array>
#include <memory>

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
    std::array<std::shared_ptr<T>, MAX_MOVES> entries;
    int                                       size = 0;

    MoveList();

    void sort();

    void rate_from_Mvvlva();

    void add_killer(Move m);
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
