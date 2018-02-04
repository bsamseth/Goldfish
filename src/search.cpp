#include <climits>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <array>
#include <vector>

#include "search.h"
#include "position.h"
#include "evaluate.h"
#include "movegen.h"
#include "types.h"

using std::cout;
using std::endl;

Searcher::Searcher(bool v) : verbose(v), nodes(0), tp_move(TABLE_SIZE, CACHE_ELASTICITY),
                             tp_score(TABLE_SIZE, CACHE_ELASTICITY),
                             futureObj(exitSignal.get_future()) {}

int Searcher::run(Position &pos) {
    return search_depth_MTD(pos, MAX_SEARCH_DEPTH);
}

bool Searcher::check_move(Position &pos, const Move &m, int &best, int depth, int gamma) {
    if (depth == 0) {
        best = std::max(best, pos.maximizingScore());
        if (best >= gamma)
            return true;
    }
    if (depth > 0 or pos.value(m) >= QS_LIMIT) {
        pos.doMove(m);
        best = std::max(best, -bound(pos, 1 - gamma, depth - 1, false));
        pos.undoMove();

        if (best >= gamma) {
            tp_move.insert(pos.hash, m);
            return true;
        }
    }
    return false;
}

int Searcher::bound(Position& pos, int gamma, int depth, bool root) {
    nodes++;

    depth = std::max(depth, 0);

    // Since king-capture is possible, always check if we have a king.
    if (pos.maximizingScore() <= -MATE_LOWER)
        return -MATE_UPPER;


    // Table lookup.
    Entry entry = {-MATE_UPPER, MATE_UPPER};
    if (tp_score.tryGet({pos.hash, depth, root}, entry)) {
        Move m;
        if (entry.lower >= gamma and (not root or tp_move.tryGet(pos.hash, m)))
            return entry.lower;
        if (entry.upper < gamma)
            return entry.upper;
    }

    // Run through moves, shortcutting when possible.
    int best = -MATE_UPPER;
    MoveGenerator mg(pos);
    mg.generateMoves();
    mg.sortMoves();
    Move killer;
    if (tp_move.tryGet(pos.hash, killer))
        mg.addKiller(killer);
    for (const Move &m : mg.getGeneratedMoves()) {
        if (stopRequested() or check_move(pos, m, best, depth, gamma)) {
            break;
        }
    }

    // Table part 2.
    if (not stopRequested()) {
        if (best >= gamma)
            tp_score.insert({pos.hash, depth, root}, {best, entry.upper});
        if (best < gamma)
            tp_score.insert({pos.hash, depth, root}, {entry.lower, best});
    }

    return best;
}

/* Iterative deepening MTD-bi search. */
int Searcher::search_depth_MTD(Position & pos, int max_depth) {
    int depth;
    Entry value;
    Move m;
    for (depth = 1; depth <= max_depth and not stopRequested(); depth++) {
        nodes = 0; // Reset node counter.

        int score, gamma;
        value = {-MATE_LOWER, MATE_UPPER};
        while (value.lower < value.upper - EVAL_ROUGHNESS and not stopRequested()) {
            gamma = (value.lower + value.upper + 1) / 2;
            score = bound(pos, gamma, depth);
            if (score >= gamma)
                value.lower = score;
            if (score < gamma)
                value.upper = score;
        }

        // Ensure move to play is in tp_move by another call
        // that must faill high and therefore produce a move.
        score = bound(pos, value.lower, depth);
        m = tp_move.get(pos.hash);
        if (verbose) {
            printf("info nodes %d depth %d score cp %d pv %s\n",
                   nodes, depth,
                   /* valueToString(value.lower*sign, pos.sideToMove).c_str(), */
                   value.lower,
                   m.str().c_str());
            std::cout << std::flush;
        }
        // If a mate is found there is no need to continue.
        if (std::abs(value.lower) >= MATE_LOWER) {
            break;
        }
    }
    if (verbose) {
        printf("bestmove %s\n", m.str().c_str());
    }
    return value.lower;
}
