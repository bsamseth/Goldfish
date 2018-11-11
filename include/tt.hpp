#pragma once

#include "position.hpp"
#include "move.hpp"
#include "depth.hpp"
#include "value.hpp"
#include "bound.hpp"
#include "move.hpp"

namespace goldfish::tt {

struct  TTEntry  {
    uint32_t key32;
    uint32_t move32;
    int16_t value16;
    uint8_t bound8;
    uint8_t depth8;

    uint32_t key() const { return key32; }
    Move move() const { return Move(move32); }
    Value value() const { return Value(value16); }
    Bound bound() const { return Bound(bound8); }
    Depth depth() const { return Depth(depth8); }


    void save(uint64_t key, Value value, Bound bound, Depth depth, Move move);
};

// If the compiler does not support packed attribute (GCC feature), then the size might not be
// as expected. We would like to know this, so fail hard.
static_assert(sizeof(TTEntry) == 12, "Size of TTEntry not as expected.");

template<int Mb_Size>
class TranspositionTable {
    private:
        static constexpr size_t size = Mb_Size * 1024 * 1024 / sizeof(TTEntry);
        std::array<TTEntry, size> table;

    public:

        const TTEntry* probe(const uint64_t key) const;
        void store(const uint64_t key, Value value, Bound bound, Depth depth, Move move);
};


template<int Mb_Size>
const TTEntry* TranspositionTable<Mb_Size>::probe(const uint64_t key) const {
    const TTEntry* tte = &table[static_cast<uint32_t>(key) % size];
    if (tte->key() == key >> 32) {
        return tte;
    }
    return nullptr;
}

template<int Mb_Size>
void TranspositionTable<Mb_Size>::store(const uint64_t key, Value value, Bound bound, Depth depth, Move move) {
    TTEntry *tte = &table[static_cast<uint32_t>(key) % size];

    // Update preference implemented in save, not handled here.
    tte->save(key, value, bound, depth, move);
}

}
