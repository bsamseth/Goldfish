#ifndef SEARCH_H
#define SEARCH_H

#include <climits>
#include <array>
#include <vector>

#include "types.h"
#include "move.h"
#include "position.h"
#include "LRUCache.h"

const int MAX_SEARCH_DEPTH = 50;
const uint64_t TABLE_SIZE = 1e8;
const uint64_t CACHE_ELASTICITY = 0;

struct Entry{
    int lower, upper;
};
struct ScoreLookupKey {
    Key hash;
    int depth;
    bool root;

    bool operator==(const ScoreLookupKey &other) const {
        return hash == other.hash and depth == other.depth and root == other.root;
    }
};
namespace std {
    template <>
    struct hash<ScoreLookupKey> {
        std::size_t operator()(const ScoreLookupKey &k) const {
            // Combine hashes using XOR and bit shifting:
            return ((k.hash ^ (std::hash<int>()(k.depth) << 1)) >> 1) ^ (hash<bool>()(k.root) << 1);
        }
    };
}

class Searcher {
    private:
        bool verbose, interrupted = false;
        int nodes;
        LRU::Cache<Key, Move> tp_move;
        LRU::Cache<ScoreLookupKey, Entry> tp_score;
        std::array<Move, MAX_SEARCH_DEPTH> pv;

        int alpha_beta_negamax(Position & pos, int depth, int alpha, int beta, int side,
                std::array<Move, MAX_SEARCH_DEPTH> & PV);
        int bound(Position& pos, int gamma, int depth, bool root = true);
        bool check_move(Position &pos, const Move &m, int &best, int depth, int gamma);

    public:
        Searcher(bool verbose = true);
        void interrupt();
        std::string pvString() const;
        int search_depth(Position & pos, int max_depth);
        int search_depth_MTD(Position & pos, int max_depth);
};

inline void Searcher::interrupt() {
    interrupted = true;
}

#endif /* SEARCH_H */
