#include <iostream>

#include "search.hpp"

namespace goldfish {

Search::Timer::Timer(bool &timer_stopped, bool &do_time_management, Depth &current_depth, const Depth &initial_depth,
                     bool &abort)
        : timer_stopped(timer_stopped), do_time_management(do_time_management),
          current_depth(current_depth), initial_depth(initial_depth), abort(abort) {
}

void Search::Timer::run(uint64_t search_time) {
    std::unique_lock<std::mutex> lock(mutex);
    if (condition.wait_for(lock, std::chrono::milliseconds(search_time)) == std::cv_status::timeout) {
        timer_stopped = true;

        // If we finished the first iteration, we should have a result.
        // In this case abort the search.
        if (!do_time_management || current_depth > initial_depth) {
            abort = true;
        }
    }
}

void Search::Timer::start(uint64_t search_time) {
    thread = std::thread(&Search::Timer::run, this, search_time);
}

void Search::Timer::stop() {
    condition.notify_all();
    thread.join();
}

Search::Semaphore::Semaphore(int permits)
        : permits(permits) {
}

void Search::Semaphore::acquire() {
    std::unique_lock<std::mutex> lock(mutex);
    while (permits == 0) {
        condition.wait(lock);
    }
    permits--;
}

void Search::Semaphore::release() {
    std::unique_lock<std::mutex> lock(mutex);
    permits++;
    condition.notify_one();
}

void Search::Semaphore::drain_permits() {
    std::unique_lock<std::mutex> lock(mutex);
    permits = 0;
}

void Search::new_depth_search(Position &position, Depth search_depth) {
    if (search_depth < 1 || search_depth > Depth::DEPTH_MAX) throw std::exception();
    if (running) throw std::exception();

    reset();

    this->position = position;
    this->search_depth = search_depth;
}

void Search::new_nodes_search(Position &position, uint64_t search_nodes) {
    if (search_nodes < 1) throw std::exception();
    if (running) throw std::exception();

    reset();

    this->position = position;
    this->search_nodes = search_nodes;
}

void Search::new_time_search(Position &position, uint64_t search_time) {
    if (search_time < 1) throw std::exception();
    if (running) throw std::exception();

    reset();

    this->position = position;
    this->search_time = search_time;
    this->run_timer = true;
}

void Search::new_infinite_search(Position &position) {
    if (running) throw std::exception();

    reset();

    this->position = position;
}

void Search::new_clock_search(Position &position,
                              uint64_t white_time_left, uint64_t white_time_increment, uint64_t black_time_left,
                              uint64_t black_time_increment, int moves_to_go) {
    new_ponder_search(position,
                      white_time_left, white_time_increment, black_time_left, black_time_increment, moves_to_go
    );

    this->run_timer = true;
}

void Search::new_ponder_search(Position &position,
                               uint64_t white_time_left, uint64_t white_time_increment, uint64_t black_time_left,
                               uint64_t black_time_increment, int moves_to_go) {
    if (white_time_left < 1) throw std::exception();
    if (black_time_left < 1) throw std::exception();
    if (moves_to_go < 0) throw std::exception();
    if (running) throw std::exception();

    reset();

    this->position = position;

    uint64_t time_left;
    uint64_t time_increment;
    if (position.active_color == Color::WHITE) {
        time_left = white_time_left;
        time_increment = white_time_increment;
    } else {
        time_left = black_time_left;
        time_increment = black_time_increment;
    }

    // Don't use all of our time. Search only for 95%. Always leave 1 second as
    // buffer time.
    uint64_t max_search_time = (uint64_t) (time_left * 0.95) - 1000L;
    if (max_search_time < 1) {
        // We don't have enough time left. Search only for 1 millisecond, meaning
        // get a result as fast as we can.
        max_search_time = 1;
    }

    // Assume that we still have to do moves_to_go number of moves. For every next
    // move (moves_to_go - 1) we will receive a time increment.
    this->search_time = (max_search_time + (moves_to_go - 1) * time_increment) / moves_to_go;
    if (this->search_time > max_search_time) {
        this->search_time = max_search_time;
    }

    this->do_time_management = true;
}

Search::Search(Protocol &protocol)
        : wakeup_signal(0), run_signal(0), stop_signal(0), finished_signal(0),
          protocol(protocol),
          timer(timer_stopped, do_time_management, current_depth, initial_depth, abort) {

    reset();

    thread = std::thread(&Search::run, this);
}

void Search::reset() {
    search_depth = Depth::DEPTH_MAX;
    search_nodes = std::numeric_limits<uint64_t>::max();
    search_time = 0;
    run_timer = false;
    timer_stopped = false;
    do_time_management = false;
    root_moves.size = 0;
    abort = false;
    total_nodes = 0;
    current_depth = initial_depth;
    current_max_depth = Depth::DEPTH_ZERO;
    current_move = Move::NO_MOVE;
    current_move_number = 0;
}

void Search::start() {
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (!running) {
        wakeup_signal.release();
        run_signal.acquire();
    }
}

void Search::stop() {
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (running) {
        // Signal the search thread that we want to stop it
        abort = true;

        stop_signal.acquire();
    }
}

void Search::ponderhit() {
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (running) {
        // Enable time management
        run_timer = true;
        timer.start(search_time);

        // If we finished the first iteration, we should have a result.
        // In this case check the stop conditions.
        if (current_depth > initial_depth) {
            check_stop_conditions();
        }
    }
}

void Search::quit() {
    std::unique_lock<std::recursive_mutex> lock(sync);

    stop();

    shutdown = true;
    wakeup_signal.release();

    thread.join();
}

void Search::wait_for_finished() {
    // Finished signal only available after run is finished.
    finished_signal.acquire();

    // Don't release again - a later call should not
    // acquire before run has drained and released again.
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
        for (Depth depth = initial_depth; depth <= search_depth; ++depth) {
            current_depth = depth;
            current_max_depth = Depth::DEPTH_ZERO;
            protocol.send_status(false, current_depth, current_max_depth, total_nodes, current_move,
                                 current_move_number);

            search_root(current_depth, -Value::INFINITE, Value::INFINITE);

            // Sort the root move list, so that the next iteration begins with the
            // best move first.
            root_moves.sort();

            check_stop_conditions();

            if (abort) {
                break;
            }
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

void Search::search_root(Depth depth, Value alpha, Value beta) {
    int ply = 0;

    update_search(ply);

    // Abort conditions
    if (abort) {
        return;
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
        Value value;
        //
        // Principal Variation Search (or really NegaScout)
        //
        // Search first move fully, then just check for moves that will
        // improve alpha using a 1-point search window. If first move was
        // best, then we will save lots of time as bounding is much faster than
        // finding exact scores. Given a good ordering (which we have due to
        // iterative deepening) the first move will be very good, and lots of
        // cutoffs can be made.
        //
        // If we find a later move that actually improves alpha, we must search this
        // properly to find its value. The idea is that this drawback is smaller than
        // the improvements gained.
        //
        if (depth > 2 and i > 0) {

            value = -search(depth - 1, -alpha - 1, -alpha, ply + 1);

            if (value > alpha and value < beta) {
                // PV search failed high, need to do a research.
                // Assuming no instability, and using the new limit.
                value = -search(depth - 1, -beta, -value, ply + 1);
            }
        }
        // First move - search fully.
        else {
            value = -search(depth - 1, -beta, -alpha, ply + 1);
        }
        position.undo_move(move);

        if (abort) {
            return;
        }

        // Do we have a better value?
        if (value > alpha) {
            alpha = value;

            // We found a new best move
            root_moves.entries[i]->value = value;
            save_pv(move, pv[ply + 1], root_moves.entries[i]->pv);

            protocol.send_move(*root_moves.entries[i], current_depth, current_max_depth, total_nodes);

            if (value >= beta)
                return;
        }
    }

    if (root_moves.size == 0) {
        // The root position is a checkmate or stalemate. We cannot search
        // further. Abort!
        abort = true;
    }
}

Value Search::search(Depth depth, Value alpha, Value beta, int ply) {
    // Check TTable before anything else is done.
    auto entry = ttable.probe(position.zobrist_key);
    if (entry != nullptr and entry->depth() >= depth) {
        switch (entry->bound()) {

            case Bound::EXACT:
                // In the case of exact scores, we can always return
                // right away. Just update best move if we exceed alpha.
                if (entry->value() > alpha)
                    save_pv(entry->move(), pv[ply + 1], pv[ply]);
                update_search(ply);
                return entry->value();

            case Bound::LOWER:
                // With a lower bound we check if we should update alpha.
                // After all this we
                if (entry->value() > alpha) {
                    save_pv(entry->move(), pv[ply + 1], pv[ply]);
                    alpha = entry->value();

                    // Check for zero-size search window.
                    if (entry->value() >= beta) {
                        update_search(ply);
                        return entry->value();
                    }
                }
                break;

            case Bound::UPPER:
                // For upper bounds, we can stop if the bound
                // is leq than alpha because no move will improve alpha.
                // If alpha < bound we have no usefull info, as the
                // exact value could still be less than alpha, so we
                // cannot update alpha and pv yet.
                if (entry->value() <= alpha) {
                    update_search(ply);
                    return entry->value();
                }
                break;
            default:
                throw std::exception();
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

    if (position.is_check())
        depth += 1;

    // Initialize
    Value best_value = -Value::INFINITE;
    Move best_move = Move::NO_MOVE;
    Bound best_value_bound = Bound::UPPER;
    int searched_moves = 0;
    bool is_check = position.is_check();

    // Null move pruning.
    // Only use when not in check, and when at least one piece is present
    // on the board. This avoids most zugzwang cases.
    if (!is_check && (
        position.pieces[position.active_color][PieceType::QUEEN] ||
        position.pieces[position.active_color][PieceType::ROOK]  ||
        position.pieces[position.active_color][PieceType::BISHOP] ||
        position.pieces[position.active_color][PieceType::KNIGHT])) {


        position.make_null_move();

        // We do recursive null move, with depth reduction factor 3.
        // Why 3? Because this is common, for instance in sunfish.
        constexpr Depth R = Depth(3);
        Value value = -search(depth - R, -beta, -beta + 1, ply + 1);

        position.undo_null_move();

        // Do not return unproven mate scores
        if (value >= Value::CHECKMATE_THRESHOLD)
            value = beta;

        // Beta cutoff?
        if (value >= beta) {
            ttable.store(position.zobrist_key, value, Bound::LOWER, depth - R, Move::NO_MOVE);
            return value;
        }
    }

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, depth, is_check);

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
            searched_moves++;
            //
            // NegaScout Search (see search_root for details).
            //
            if (depth > 1 and i > 0) {

                value = -search(depth - 1, -alpha - 1, -alpha, ply + 1);

                if (value > alpha and value < beta) {
                    // PV search failed high, need to do a research.
                    // Assuming no instability, and using the new limit.
                    value = -search(depth - 1, -beta, -value, ply + 1);
                }
            } else {
                // First move - do full search.
                value = -search(depth - 1, -beta, -alpha, ply + 1);
            }
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
                best_value_bound = Bound::EXACT;
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta) {
                    best_value_bound = Bound::LOWER;
                    // Cut-off
                    break;
                }
            }
        }
    }

    // If we cannot move, check for checkmate and stalemate.
    if (searched_moves == 0) {
        Value return_value = is_check ? -Value::CHECKMATE + ply
                                      : Value::DRAW;
        ttable.store(position.zobrist_key, return_value, Bound::EXACT,
                     Depth::DEPTH_MAX, Move::NO_MOVE);
        return return_value;
    }

    ttable.store(position.zobrist_key, best_value, best_value_bound,
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
    Move best_move = Move::NO_MOVE;
    Bound best_value_bound = Bound::UPPER;
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
                ttable.store(position.zobrist_key, best_value, Bound::LOWER, Depth::DEPTH_ZERO, Move::NO_MOVE);
                return best_value;
            }
        }
    }
    //### ENDOF Stand pat

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, Depth::DEPTH_ZERO, is_check);
    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;
        Value value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            searched_moves++;
            // Note that we do not use PVS/NegaScout here, as we have no
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
            best_move = move;

            // Do we have a better value?
            if (value > alpha) {
                best_value_bound = Bound::EXACT;
                alpha = value;
                save_pv(move, pv[ply + 1], pv[ply]);

                // Is the value higher than beta?
                if (value >= beta) {
                    // Cut-off
                    best_value_bound = Bound::LOWER;
                    break;
                }
            }
        }
    }

    // If we cannot move, check for checkmate.
    if (searched_moves == 0 && is_check) {
        // We have a check mate. This is bad for us, so return a -CHECKMATE.
        Value return_value = -Value::CHECKMATE + ply;
        ttable.store(position.zobrist_key, return_value, Bound::EXACT, Depth::DEPTH_MAX, Move::NO_MOVE);
        return return_value;
    }

    ttable.store(position.zobrist_key, best_value, best_value_bound, Depth::DEPTH_ZERO, best_move);
    return best_value;
}

void Search::save_pv(Move move, MoveVariation &src, MoveVariation &dest) {
    dest.moves[0] = move;
    for (int i = 0; i < src.size; i++) {
        dest.moves[i + 1] = src.moves[i];
    }
    dest.size = src.size + 1;
}
uint64_t Search::get_total_nodes() {
    return total_nodes;
}

}
