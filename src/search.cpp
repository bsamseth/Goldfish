#include <iostream>

#include "search.hpp"

namespace goldfish {

void Search::check_stop_conditions() {
    // We will check the stop conditions only if we are using time management,
    // that is if our timer != null.
    if (run_timer && do_time_management) {
        if (timer_stopped) {
            abort = true;
        } else {
            // Check if we have only one move to make
            if (root_moves.size == 1) {
                abort = true;
            } else

                // Check if we have a checkmate
            if (Values::is_checkmate(root_moves.entries[0]->value)
                && current_depth >= Depth(Value::CHECKMATE - std::abs(root_moves.entries[0]->value))) {
                abort = true;
            }
        }
    }
}

void Search::update_search(int ply) {
    total_nodes++;

    if (ply > current_max_depth) {
        current_max_depth = Depth(ply);
    }

    if (search_nodes <= total_nodes) {
        // Hard stop on number of nodes
        abort = true;
    }

    pv[ply].size = 0;

    protocol.send_status(current_depth, current_max_depth, total_nodes, current_move, current_move_number);
}

void Search::run() {
    while (true) {
        wakeup_signal.acquire();

        if (shutdown) {
            break;
        }

        // Do all initialization before releasing the main thread to JCPI
        if (run_timer) {
            timer.start(search_time);
        }

        // Populate root move list
        MoveList<MoveEntry> &moves = move_generators[0].get_legal_moves(position, 1, position.is_check());
        for (int i = 0; i < moves.size; i++) {
            Move move = moves.entries[i]->move;
            root_moves.entries[root_moves.size]->move = move;
            root_moves.entries[root_moves.size]->pv.moves[0] = move;
            root_moves.entries[root_moves.size]->pv.size = 1;
            root_moves.size++;
        }

        // Go...
        finished_signal.drain_permits();
        stop_signal.drain_permits();
        running = true;
        run_signal.release();

        //### BEGIN Iterative Deepening
        for (Depth depth = initial_depth; !abort and depth <= search_depth; ++depth) {
            current_depth = depth;
            current_max_depth = Depth::DEPTH_ZERO;
            protocol.send_status(false, current_depth, current_max_depth, total_nodes, current_move,
                                 current_move_number);

            search_root(depth, -Value::INFINITE, Value::INFINITE);

            // Sort the root move list, so that the next iteration begins with the
            // best move first.
            root_moves.sort();

            check_stop_conditions();
        }
        //### ENDOF Iterative Deepening

        if (run_timer) {
            timer.stop();
        }

        // Update all stats
        protocol.send_status(true, current_depth, current_max_depth, total_nodes, current_move, current_move_number);

        // Send the best move and ponder move
        Move best_move = Move::NO_MOVE;
        Move ponder_move = Move::NO_MOVE;
        if (root_moves.size > 0) {
            best_move = root_moves.entries[0]->move;
            if (root_moves.entries[0]->pv.size >= 2) {
                ponder_move = root_moves.entries[0]->pv.moves[1];
            }
        }

        // Send the best move to the GUI
        protocol.send_best_move(best_move, ponder_move);

        running = false;
        stop_signal.release();
        finished_signal.release();
    }
}

/*
 Principal Variation Search

 Search first move fully, then just check for moves that will
 improve alpha using a 1-point search window. If first move was
 best, then we will save lots of time as bounding is much faster than
 finding exact scores. Given a good ordering (which we have due to
 iterative deepening) the first move will be very good, and lots of
 cutoffs can be made.

 If we find a later move that actually improves alpha, we must search this
 properly to find its value. The idea is that this drawback is smaller than
 the improvements gained.
*/
Value Search::pv_search(Depth depth, Value alpha, Value beta, int ply, int move_number) {
    if (depth > 1 and move_number > 0) {

        Value value = -search(depth - 1, -alpha - 1, -alpha, ply + 1);

        if (value <= alpha)
            return value;
    }

    return -search(depth - 1, -beta, -alpha, ply + 1);
}


Value Search::search_root(Depth depth, Value alpha, Value beta) {
    int ply = 0;

    update_search(ply);

    Value best_value = -Value::INFINITE;

    // Abort conditions
    if (abort) {
        return best_value;
    }

    if (root_moves.size == 0) {
        // The root position is a checkmate or stalemate. We cannot search
        // further. Abort!
        abort = true;
        return position.is_check() ? -Value::CHECKMATE + ply
                                   :  Value::DRAW;
    }

    // Reset all values, so the best move is pushed to the front
    for (int i = 0; i < root_moves.size; i++) {
        root_moves.entries[i]->value = -Value::INFINITE;
    }

    for (int i = 0; i < root_moves.size; i++) {
        Move move = root_moves.entries[i]->move;

        current_move = move;
        current_move_number = i + 1;
        protocol.send_status(false, current_depth, current_max_depth, total_nodes, current_move, current_move_number);

        position.make_move(move);

        Value value = pv_search(depth, alpha, beta, ply, i);

        position.undo_move(move);

        if (abort) {
            return best_value;
        }

        best_value = std::max(best_value, value);

        // Do we have a better value?
        if (value > alpha) {
            alpha = value;

            // We found a new best move
            root_moves.entries[i]->value = value;
            save_pv(move, pv[ply + 1], root_moves.entries[i]->pv);

            protocol.send_move(*root_moves.entries[i], current_depth, current_max_depth, total_nodes);

            if (value >= beta)
                return value;
        }
    }

    assert(best_value > -Value::INFINITE);
    return best_value;
}

Value Search::search(Depth depth, Value alpha, Value beta, int ply) {
    Value alpha_orig = alpha;

    // Check TTable before anything else is done.
    auto entry = ttable.probe(position.zobrist_key);
    if (entry != nullptr and entry->depth() >= depth) {
        const Value tt_value = tt::value_from_tt(entry->value(), ply);
        if (entry->bound() & Bound::LOWER) {
            if (tt_value > alpha) {
                save_pv(entry->move(), pv[ply + 1], pv[ply]);
                alpha = tt_value;
            }
        }
        if (entry->bound() & Bound::UPPER) {
            if (tt_value < beta)
                beta = tt_value;
        }
        if (alpha >= beta) {
            update_search(ply);
            return tt_value;
        }
    }

    // We are at a leaf/horizon. So calculate that value.
    if (depth <= 0) {
        // Descend into quiescent
        return quiescent(alpha, beta, ply);
    }

    update_search(ply);

    // Abort conditions
    if (abort || ply == Depth::MAX_PLY) {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.is_repetition() || position.has_insufficient_material() || position.halfmove_clock >= 100) {
        return Value::DRAW;
    }

    // Mate distance pruning:
    // Even if we mate at the next move our score
    // would be at best CHECKMATE - ply, but if alpha is already bigger because
    // a shorter mate was found upward in the tree then there is no need to search
    // because we will never beat the current alpha. Same logic but with reversed
    // signs applies also in the opposite condition of being mated instead of giving
    // mate. In this case return a fail-high score.
    alpha = std::max(-Value::CHECKMATE + ply, alpha);
    beta = std::min(Value::CHECKMATE - (ply + 1), beta);
    if (alpha >= beta)
        return alpha;

    bool is_check = position.is_check();


    // Null move pruning.
    //
    // Idea is that if we have a position that is so strong that even if we
    // don't move (i.e. pass), we still exceed beta. Only started if we have reason
    // to belive that NMP will be useful, which here is check by eval >= beta.
    //
    // Only used when the following is _NOT_ true:
    //
    //  1. We are in check (NM would be illegal)
    //  2. The last move made was a null move (double null move has no effect other than reduced depth)
    //  3. A beta-cutoff must be by finding a mate score (mates with NM is not proven)
    //  4. We are in zugzwang (not moving is better than any other move)
    //
    // Number 4 is hard to guarantee (but possible with verification search, see SF).
    // But by not using null move when we only have K and P we escape most cases.
    if (!is_check &&
        beta < Value::CHECKMATE_THRESHOLD &&
        !position.last_move_was_null_move() && (
        position.pieces[position.active_color][PieceType::QUEEN] ||
        position.pieces[position.active_color][PieceType::ROOK]  ||
        position.pieces[position.active_color][PieceType::BISHOP] ||
        position.pieces[position.active_color][PieceType::KNIGHT]) &&
        Evaluation::evaluate(position) >= beta) {


        position.make_null_move();
        assert(position.last_move_was_null_move());

        // We do recursive null move, with depth reduction factor 3.
        // Why 3? Because this is common, for instance in sunfish.
        constexpr Depth R = Depth(3);
        Value value = -search(depth - R, -beta, -beta + 1, ply + 1);

        position.undo_null_move();
        assert(!position.last_move_was_null_move());


        // Beta cutoff?
        if (value >= beta) {

            // Do not return unproven mate scores
            if (value >= Value::CHECKMATE_THRESHOLD)
                value = beta;

            ttable.store(position.zobrist_key, tt::value_to_tt(value, ply), Bound::LOWER, std::max(Depth::DEPTH_ZERO, depth - R + 1), Move::NO_MOVE);
            return value;
        }
    }

    // Initialize
    Value best_value = -Value::INFINITE;
    Move best_move = Move::NO_MOVE;
    int searched_moves = 0;

    if (is_check)
        depth += 1;

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, depth, is_check);

    // Internal Iterative deepening:
    // When we have no good guess for the best move, do a reduced search
    // first to find a likely candidate. Only do this if a search would
    // lead to a new entry in the ttable.
    constexpr Depth iid_reduction = Depth(7);
    if (     depth > iid_reduction
        and (entry == nullptr
         or (entry->move() == Move::NO_MOVE and entry->depth() < depth - iid_reduction))) {

        search(depth - iid_reduction, alpha, beta, ply);
        entry = ttable.probe(position.zobrist_key);
    }

    // Killer Move Heuristic:
    // If lookup didn't cause a cutoff, including if we don't have the required depth to use
    // the table entry, lets use the stored move as a killer move,
    // searching it first in the hopes that it will lead to more cutoffs.
    if (entry != nullptr and entry->move() != Move::NO_MOVE) {
        moves.add_killer(entry->move());
    }

    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;
        Value value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {

            value = pv_search(depth, alpha, beta, ply, searched_moves);

            searched_moves++;
        }
        position.undo_move(move);

        if (abort) {
            return best_value;
        }

        // Pruning
        if (value > best_value) {
            best_value = value;
            best_move = move;

            // Do we have a better value?
            if (value > alpha) {
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta) {
                    // Cut-off
                    break;
                }
            }
        }
    }


    // Determine bound type.
    Bound best_value_bound = best_value <= alpha_orig ? Bound::UPPER :
                                best_value >= beta ? Bound::LOWER :
                                                     Bound::EXACT;

    // If we cannot move, check for checkmate and stalemate.
    if (searched_moves == 0) {
        best_value = is_check ? -Value::CHECKMATE + ply : Value::DRAW;
        best_value_bound = Bound::EXACT;
    }

    ttable.store(position.zobrist_key, tt::value_to_tt(best_value, ply), best_value_bound,
                 depth, best_move);
    return best_value;
}

Value Search::quiescent(Value alpha, Value beta, int ply) {
    // No need to check the ttable, as we only decend to quiescense if there is
    // no entry in the table.

    update_search(ply);

    // Abort conditions
    if (abort || ply == Depth::MAX_PLY) {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.is_repetition() || position.has_insufficient_material() || position.halfmove_clock >= 100) {
        return Value::DRAW;
    }

    // Initialize
    Value best_value = -Value::INFINITE;
    int searched_moves = 0;
    bool is_check = position.is_check();

    //### BEGIN Stand pat
    if (!is_check) {
        best_value = Evaluation::evaluate(position);

        // Do we have a better value?
        if (best_value > alpha) {
            alpha = best_value;

            // Is the value higher than beta?
            if (best_value >= beta) {
                // Cut-off
                return best_value;
            }
        }
    }
    //### ENDOF Stand pat

    // Delta pruning:
    // Test if alpha can be improved by greatest
    // possible material swing. If not, then don't bother.
    //
    // Best possible single move is to capture a queen while promoting a pawn.
    // Make sure we're not in check, as then the stand pat is -INFINITE.
    Value delta = 2 * Value::QUEEN_VALUE - Value::PAWN_VALUE;
    if (!is_check and best_value + delta < alpha)
        return best_value;

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, Depth::DEPTH_ZERO, is_check);
    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;
        Value value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            searched_moves++;
            // Note that we do not use PVS here, as we have no
            // reason to believe move ordering works very well here, and
            // we know we don't have a killer move from ttable.
            value = -quiescent(-beta, -alpha, ply + 1);
        }
        position.undo_move(move);

        if (abort) {
            return best_value;
        }

        // Pruning
        if (value > best_value) {
            best_value = value;

            // Do we have a better value?
            if (value > alpha) {
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta) {
                    // Cut-off
                    break;
                }
            }
        }
    }

    // If we cannot move, check for checkmate.
    if (searched_moves == 0 && is_check) {
        // We have a check mate. This is bad for us, so return a -CHECKMATE.
        return - Value::CHECKMATE + ply;
    }

    return best_value;
}


}
