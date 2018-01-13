#ifndef SEARCH_H
#define SEARCH_H

#include <climits>
#include <array>
#include <vector>

#include "position.h"

const int MAX_SEARCH_DEPTH = 50;

class Searcher {
    private:
    std::array<Move, MAX_SEARCH_DEPTH> pv;
    int alpha_beta_negamax(Position & pos, int depth, int alpha, int beta, int side, std::array<Move, MAX_SEARCH_DEPTH> & PV);
    bool verbose;
    int nodes;

    public:
    Searcher();
    Searcher(bool verbose);
    std::string pvString() const;
    int search_depth(Position & pos, int max_depth);
};

#endif /* SEARCH_H */
