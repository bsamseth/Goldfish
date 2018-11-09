#pragma once

#include "position.hpp"
#include "move.hpp"
#include "depth.hpp"
#include "value.hpp"
#include "bound.hpp"
#include "move.hpp"

namespace goldfish {

struct __attribute__ ((packed)) TTEntry  {
    uint32_t key;
    Value value;
    Bound bound;
    Depth depth;
    Move move;

    void save(uint32_t key, Value value, Bound bound, Depth depth, Move move) {
        this->key = key;
        this->value = value;
        this->bound = bound;
        this->depth = depth;
        this->move = move;
    }
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

        const TTEntry* probe(const uint64_t key) const {
            const TTEntry* tte = &table[static_cast<uint32_t>(key) % size];
            if (tte->key == key >> 32) {
                return tte;
            }
            return nullptr;
        }

        void store(const uint64_t key, Value value, Bound bound, Depth depth, Move move) {
            TTEntry *tte = &table[static_cast<uint32_t>(key) % size];

            // Store new entry if depth is greater than existing entry.
            if (tte->depth < depth) {
                tte->save(key >> 32, value, bound, depth, move);
            }
        }
};

}
