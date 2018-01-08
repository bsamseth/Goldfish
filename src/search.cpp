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

Searcher::Searcher() : verbose(true), nodes(0) {}

Searcher::Searcher(bool v) : verbose(v), nodes(0) {}

/* Convert evaluation to string, handling output of mate. */
std::string valueToString(int value, Color side) {
    std::stringstream ss;
    if (std::abs(value) >= MATE_LOWER) {
        ss << "M" << (MATE_UPPER - std::abs(value) - 2) / 2 + 1;
    } else {
        ss << value;
    }
    return ss.str();
}

/* Iterative deepening */
int Searcher::search_depth(Position & pos, int max_depth) {
    int value, depth;
    int sign = pos.sideToMove == WHITE ? 1 : -1;
    for (depth = 1; depth <= max_depth; depth++) {
        nodes = 0; // Reset node counter.
        value = sign * alpha_beta_negamax(pos, depth, -MATE_UPPER, MATE_UPPER, sign, pv);
        if (verbose) {
            printf("info nodes %d depth %d score cp %s pv %s\n",
                   nodes, depth,
                   valueToString(value, pos.sideToMove).c_str(),
                   pvString().c_str());
        }
        // If a mate is found there is no need to continue.
        if (std::abs(value) >= MATE_LOWER) {
            break;
        }
    }
    if (verbose) {
        printf("bestmove %s\n", pvString().substr(0, 5).c_str());
    }
    return value;
}

int Searcher::alpha_beta_negamax(Position & pos, int depth, int alpha, int beta, int side, std::array<Move, MAX_SEARCH_DEPTH> & PV) {
    nodes++;
    if (side * pos.score() <= -MATE_LOWER) {
        return - MATE_UPPER;
    }
    if (depth == 0) {
        return side * pos.score();
    }
    int best_value = - MATE_UPPER;
    std::array<Move, MAX_SEARCH_DEPTH> PV_tmp;
    MoveGenerator mg = MoveGenerator(pos);
    mg.generateMoves();
    for (Move m : mg.getGeneratedMoves()) {
        pos.doMove(m);
        int v = -alpha_beta_negamax(pos, depth - 1, -beta, -alpha, -side, PV_tmp);
        v += v > MATE_LOWER ? -1 :
             v < -MATE_LOWER ? 1 : 0;
        pos.undoMove();

        if (v > best_value) {
            best_value = v;
            PV[depth] = m;
            for (int i = 0; i < depth; i++) {
                PV[i] = PV_tmp[i];
            }
        }
        alpha = std::max(alpha, v);
        if (alpha >= beta) {
            break;
        }
    }
    return best_value;
}

std::string Searcher::pvString() {
    std::stringstream ss;
    for (int depth = pv.size() - 1; depth >= 0; depth--) {
        Move &m = pv[depth];
        if (m.getInteger() != NO_MOVE) {
            ss << m.str() << " ";
        }
    }
    return ss.str();
}

