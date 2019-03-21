#include "search.hpp"

#include <iostream>

namespace goldfish
{
void Search::new_depth_search(Position& position, Depth search_depth)
{
    if (search_depth < 1 || search_depth > Depth::DEPTH_MAX)
        throw std::exception();
    if (running)
        throw std::exception();

    reset();

    this->position     = position;
    this->search_depth = search_depth;
}

void Search::new_nodes_search(Position& position, uint64_t search_nodes)
{
    if (search_nodes < 1)
        throw std::exception();
    if (running)
        throw std::exception();

    reset();

    this->position     = position;
    this->search_nodes = search_nodes;
}

void Search::new_time_search(Position& position, uint64_t search_time)
{
    if (search_time < 1)
        throw std::exception();
    if (running)
        throw std::exception();

    reset();

    this->position    = position;
    this->search_time = search_time;
    this->run_timer   = true;
}

void Search::new_infinite_search(Position& position)
{
    if (running)
        throw std::exception();

    reset();

    this->position = position;
}

void Search::new_clock_search(Position& position,
                              uint64_t  white_time_left,
                              uint64_t  white_time_increment,
                              uint64_t  black_time_left,
                              uint64_t  black_time_increment,
                              int       moves_to_go)
{
    new_ponder_search(position,
                      white_time_left,
                      white_time_increment,
                      black_time_left,
                      black_time_increment,
                      moves_to_go);

    this->run_timer = true;
}

void Search::new_ponder_search(Position& position,
                               uint64_t  white_time_left,
                               uint64_t  white_time_increment,
                               uint64_t  black_time_left,
                               uint64_t  black_time_increment,
                               int       moves_to_go)
{
    if (white_time_left < 1)
        throw std::exception();
    if (black_time_left < 1)
        throw std::exception();
    if (moves_to_go < 0)
        throw std::exception();
    if (running)
        throw std::exception();

    reset();

    this->position = position;

    uint64_t time_left;
    uint64_t time_increment;
    if (position.active_color == Color::WHITE)
    {
        time_left      = white_time_left;
        time_increment = white_time_increment;
    }
    else
    {
        time_left      = black_time_left;
        time_increment = black_time_increment;
    }

    // Don't use all of our time. Search only for 95%. Always leave 1 second as
    // buffer time.
    uint64_t max_search_time = (uint64_t)(time_left * 0.95) - 1000L;
    if (max_search_time < 1)
    {
        // We don't have enough time left. Search only for 1 millisecond, meaning
        // get a result as fast as we can.
        max_search_time = 1;
    }

    // Assume that we still have to do moves_to_go number of moves. For every next
    // move (moves_to_go - 1) we will receive a time increment.
    this->search_time
        = (max_search_time + (moves_to_go - 1) * time_increment) / moves_to_go;
    if (this->search_time > max_search_time)
    {
        this->search_time = max_search_time;
    }

    this->do_time_management = true;
}

Search::Search(Protocol& protocol)
    : wakeup_signal(0)
    , run_signal(0)
    , stop_signal(0)
    , finished_signal(0)
    , protocol(protocol)
    , timer(timer_stopped, do_time_management, current_depth, initial_depth, abort)
{
    reset();
    thread = std::thread(&Search::run, this);
}

void Search::reset()
{
    search_depth        = Depth::DEPTH_MAX;
    search_nodes        = std::numeric_limits<uint64_t>::max();
    search_time         = 0;
    run_timer           = false;
    timer_stopped       = false;
    do_time_management  = false;
    root_moves.size     = 0;
    abort               = false;
    total_nodes         = 0;
    tb_hits             = 0;
    root_in_TB          = false;
    current_depth       = initial_depth;
    current_max_depth   = Depth::DEPTH_ZERO;
    current_move        = Move::NO_MOVE;
    current_move_number = 0;
}

void Search::start()
{
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (!running)
    {
        wakeup_signal.release();
        run_signal.acquire();
    }
}

void Search::stop()
{
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (running)
    {
        // Signal the search thread that we want to stop it
        abort = true;

        stop_signal.acquire();
    }
}

void Search::ponderhit()
{
    std::unique_lock<std::recursive_mutex> lock(sync);

    if (running)
    {
        // Enable time management
        run_timer = true;
        timer.start(search_time);

        // If we finished the first iteration, we should have a result.
        // In this case check the stop conditions.
        if (current_depth > initial_depth)
        {
            check_stop_conditions();
        }
    }
}

void Search::quit()
{
    std::unique_lock<std::recursive_mutex> lock(sync);

    stop();

    shutdown = true;
    wakeup_signal.release();

    thread.join();
}

void Search::wait_for_finished()
{
    // Finished signal only available after run is finished.
    finished_signal.acquire();

    // Don't release again - a later call should not
    // acquire before run has drained and released again.
}

}  // namespace goldfish
