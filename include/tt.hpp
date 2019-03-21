#pragma once

#include "bound.hpp"
#include "depth.hpp"
#include "move.hpp"
#include "position.hpp"
#include "value.hpp"

#include <iostream>

namespace goldfish::tt
{
struct TTEntry
{
    uint32_t key32;
    uint32_t move32;
    int16_t  value16;
    uint8_t  bound8;
    uint8_t  depth8;

    uint32_t key() const { return key32; }
    Move     move() const { return Move(move32); }
    Value    value() const { return Value(value16); }
    Bound    bound() const { return Bound(bound8); }
    Depth    depth() const { return Depth(depth8); }

    void save(uint64_t key, Value value, Bound bound, Depth depth, Move move);
};

// If the compiler does not support packed attribute (GCC feature), then the size might
// not be as expected. We would like to know this, so fail hard.
static_assert(sizeof(TTEntry) == 12, "Size of TTEntry not as expected.");

class TranspositionTable
{
private:
    static constexpr size_t MB = 1024 * 1024 / sizeof(TTEntry);
    std::vector<TTEntry>    table_;

public:
    TranspositionTable();
    TranspositionTable(size_t size);

    void           resize(size_t size);
    const TTEntry* probe(const uint64_t key) const;
    void   store(const uint64_t key, Value value, Bound bound, Depth depth, Move move);
    size_t size() const;
};

inline size_t TranspositionTable::size() const
{
    return table_.size() / MB;
}

inline void TranspositionTable::resize(size_t size)
{
    std::cout << "info string resizing transposition table to " << size << " MB\n";
    table_.resize(size * MB);
}

inline const TTEntry* TranspositionTable::probe(const uint64_t key) const
{
    const TTEntry* tte = &table_[static_cast<uint32_t>(key) % table_.size()];
    if (tte->key() == key >> 32)
    {
        return tte;
    }
    return nullptr;
}

inline void TranspositionTable::store(const uint64_t key,
                                      Value          value,
                                      Bound          bound,
                                      Depth          depth,
                                      Move           move)
{
    TTEntry* tte = &table_[static_cast<uint32_t>(key) % table_.size()];

    // Update preference implemented in save, not handled here.
    tte->save(key, value, bound, depth, move);
}

// value_to_tt() adjusts a mate score from "plies to mate from the root" to
// "plies to mate from the current position". Non-mate scores are unchanged.
// The function is called before storing a value in the transposition table.
constexpr Value value_to_tt(Value v, int ply)
{
    assert(v != Value::NO_VALUE);
    return v >= Value::CHECKMATE_THRESHOLD
               ? v + ply
               : v <= Value::CHECKMATE_THRESHOLD ? v - ply : v;
}

// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
// from the transposition table (which refers to the plies to mate/be mated
// from current position) to "plies to mate/be mated from the root".
constexpr Value value_from_tt(Value v, int ply)
{
    return v == Value::NO_VALUE ? Value::NO_VALUE
                                : v >= Value::CHECKMATE_THRESHOLD
                                      ? v - ply
                                      : v <= Value::CHECKMATE_THRESHOLD ? v + ply : v;
}
}  // namespace goldfish::tt
namespace goldfish
{
// Global TT object.
extern tt::TranspositionTable TT;
}  // namespace goldfish
