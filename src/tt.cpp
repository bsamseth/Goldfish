#include "tt.hpp"

#include "uci.hpp"

#include <cassert>

namespace goldfish
{
// Global TT object.
tt::TranspositionTable TT;

namespace tt
{
TranspositionTable::TranspositionTable() : TranspositionTable(16) {}
TranspositionTable::TranspositionTable(size_t MB_size) : table_(MB_size * MB) {}

void TTEntry::save(uint64_t key, Value value, Bound bound, Depth depth, Move move)
{
    // If we have the same position, only overwrite if move is not none.
    if (move != Move::NO_MOVE || (key >> 32) != key32)
    {
        move32 = (uint32_t) move;
    }

    // Overwrite less valueable entries.
    if ((key >> 32) != key32 || depth >= depth8)
    {
        key32 = (uint32_t)(key >> 32);

        // In debug mode, ensure that value is not overflowing 16 bits.
        assert(value < (1 << 15) and value > -(1 << 15));
        value16 = (int16_t) value;

        bound8 = (uint8_t) bound;

        assert(depth < (1 << 8));
        depth8 = (uint8_t) depth;
    }
}
}  // namespace tt
}  // namespace goldfish
