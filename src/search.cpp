#include <iostream>

#include "search.hpp"

namespace goldfish {

Search::Timer::Timer(bool &timer_stopped, bool &do_time_management, int &current_depth, const int &initial_depth,
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

void Search::new_depth_search(Position &position, int search_depth) {
    if (search_depth < 1 || search_depth > Depth::MAX_DEPTH) throw std::exception();
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
    if (white_time_increment < 0) throw std::exception();
    if (black_time_left < 1) throw std::exception();
    if (black_time_increment < 0) throw std::exception();
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
        : protocol(protocol),
          timer(timer_stopped, do_time_management, current_depth, initial_depth, abort),
          wakeup_signal(0), run_signal(0), stop_signal(0), finished_signal(0) {

    reset();

    thread = std::thread(&Search::run, this);
}

void Search::reset() {
    search_depth = Depth::MAX_DEPTH;
    search_nodes = std::numeric_limits<uint64_t>::max();
    search_time = 0;
    run_timer = false;
    timer_stopped = false;
    do_time_management = false;
    root_moves.size = 0;
    abort = false;
    total_nodes = 0;
    current_depth = initial_depth;
    current_max_depth = 0;
    current_move = Moves::NO_MOVE;
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
        for (int depth = initial_depth; depth <= search_depth; depth++) {
            current_depth = depth;
            current_max_depth = 0;
            protocol.send_status(false, current_depth, current_max_depth, total_nodes, current_move,
                                 current_move_number);

            search_root(current_depth, -Value::VALUE_INFINITE, Value::VALUE_INFINITE);

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
        int best_move = Moves::NO_MOVE;
        int ponder_move = Moves::NO_MOVE;
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
                && current_depth >= (Value::VALUE_MATE - std::abs(root_moves.entries[0]->value))) {
                abort = true;
            }
        }
    }
}

void Search::update_search(int ply) {
    total_nodes++;

    if (ply > current_max_depth) {
        current_max_depth = ply;
    }

    if (search_nodes <= total_nodes) {
        // Hard stop on number of nodes
        abort = true;
    }

    pv[ply].size = 0;

    protocol.send_status(current_depth, current_max_depth, total_nodes, current_move, current_move_number);
}

void Search::search_root(int depth, int alpha, int beta) {
    int ply = 0;

    update_search(ply);

    // Abort conditions
    if (abort) {
        return;
    }

    // Reset all values, so the best move is pushed to the front
    for (int i = 0; i < root_moves.size; i++) {
        root_moves.entries[i]->value = -Value::VALUE_INFINITE;
    }


    for (int i = 0; i < root_moves.size; i++) {
        Move move = root_moves.entries[i]->move;

        current_move = move;
        current_move_number = i + 1;
        protocol.send_status(false, current_depth, current_max_depth, total_nodes, current_move, current_move_number);

        position.make_move(move);
        int value = -search(depth - 1, -beta, -alpha, ply + 1);
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
        }
    }

    if (root_moves.size == 0) {
        // The root position is a checkmate or stalemate. We cannot search
        // further. Abort!
        abort = true;
    }
}

int Search::search(int depth, int alpha, int beta, int ply) {
    // We are at a leaf/horizon. So calculate that value.
    if (depth <= 0) {
        // Descend into quiescent
        return quiescent(0, alpha, beta, ply);
    }

    update_search(ply);

    // Abort conditions
    if (abort || ply == Depth::MAX_PLY) {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.is_repetition() || position.has_insufficient_material() || position.halfmove_clock >= 100) {
        return Value::VALUE_DRAW;
    }

    if (position.is_check())
        depth += 1;

    // Initialize
    int best_value = -Value::VALUE_INFINITE;
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
        int value = -search(depth - 3, -beta, -alpha, ply + 1);

        position.undo_null_move();

        // The value is at worst equal to best_value's initial value.
        best_value = value;

        // New best move?
        if (value > alpha) {
            alpha = value;

            // Beta cutoff?
            if (value >= beta)
                return best_value;

        }
    }

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, depth, is_check);
    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;
        int value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            searched_moves++;
            value = -search(depth - 1, -beta, -alpha, ply + 1);
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

    // If we cannot move, check for checkmate and stalemate.
    if (searched_moves == 0) {
        if (is_check) {
            // We have a check mate. This is bad for us, so return a -CHECKMATE.
            return -Value::VALUE_MATE + ply;
        } else {
            // We have a stale mate. Return the draw value.
            return Value::VALUE_DRAW;
        }
    }

    return best_value;
}

int Search::quiescent(int depth, int alpha, int beta, int ply) {
    update_search(ply);

    // Abort conditions
    if (abort || ply == Depth::MAX_PLY) {
        return Evaluation::evaluate(position);
    }

    // Check insufficient material, repetition and fifty move rule
    if (position.is_repetition() || position.has_insufficient_material() || position.halfmove_clock >= 100) {
        return Value::VALUE_DRAW;
    }

    // Initialize
    int best_value = -Value::VALUE_INFINITE;
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

    MoveList<MoveEntry> &moves = move_generators[ply].get_moves(position, depth, is_check);
    for (int i = 0; i < moves.size; i++) {
        Move move = moves.entries[i]->move;
        int value = best_value;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            searched_moves++;
            value = -quiescent(depth - 1, -beta, -alpha, ply + 1);
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
        return -Value::VALUE_MATE + ply;
    }

    return best_value;
}

void Search::save_pv(int move, MoveVariation &src, MoveVariation &dest) {
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
