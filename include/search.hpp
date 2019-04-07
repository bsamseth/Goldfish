#pragma once

#include "evaluation.hpp"
#include "movegenerator.hpp"
#include "position.hpp"
#include "protocol.hpp"
#include "tt.hpp"
#include "uci.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <array>

namespace goldfish
{
// Stack struct keeps track of the information we need to remember from nodes
// shallower and deeper in the tree during the search. The number of entries here
// will likely grow as search becomes more sophisticated.
struct Stack
{
    Value staticEval;
    std::array<Move, 2> killers = {Move::NO_MOVE, Move::NO_MOVE};

    explicit Stack(Value v = Value::NO_VALUE) : staticEval(v) {}
};

/**
 * This class implements our search in a separate thread to keep the main
 * thread available for more commands.
 */
class Search
{
public:
    explicit Search(Protocol& protocol);

    void new_depth_search(Position& position, Depth search_depth);

    void new_nodes_search(Position& position, uint64_t search_nodes);

    void new_time_search(Position& position, uint64_t search_time);

    void new_infinite_search(Position& position);

    void new_clock_search(Position& position,
                          uint64_t  white_time_left,
                          uint64_t  white_time_increment,
                          uint64_t  black_time_left,
                          uint64_t  black_time_increment,
                          int       moves_to_go);

    void new_ponder_search(Position& position,
                           uint64_t  white_time_left,
                           uint64_t  white_time_increment,
                           uint64_t  black_time_left,
                           uint64_t  black_time_increment,
                           int       moves_to_go);

    void reset();

    void start();

    void stop();

    void ponderhit();

    void quit();

    void wait_for_finished();

    void run();

    uint64_t get_total_nodes();

private:
    /**
     * This is our search timer for time & clock & ponder searches.
     */
    class Timer
    {
    public:
        Timer(bool&        timer_stopped,
              bool&        do_time_management,
              Depth&       current_depth,
              const Depth& initial_depth,
              bool&        abort);

        void start(uint64_t search_time);

        void stop();

    private:
        std::mutex              mutex;
        std::condition_variable condition;
        std::thread             thread;

        bool&        timer_stopped;
        bool&        do_time_management;
        Depth&       current_depth;
        const Depth& initial_depth;

        bool& abort;

        void run(uint64_t search_time);
    };

    class Semaphore
    {
    public:
        explicit Semaphore(int permits);

        void acquire();

        void release();

        void drain_permits();

    private:
        int                     permits;
        std::mutex              mutex;
        std::condition_variable condition;
    };

    std::thread          thread;
    Semaphore            wakeup_signal;
    Semaphore            run_signal;
    Semaphore            stop_signal;
    Semaphore            finished_signal;
    std::recursive_mutex sync;
    Protocol&            protocol;
    bool                 running  = false;
    bool                 shutdown = false;

    Position position;

    // We will store a MoveGenerator for each ply so we don't have to create them
    // in search. (which is expensive)
    std::array<MoveGenerator, Depth::MAX_PLY> move_generators;

    // Similarly for the search Stacks
    std::array<Stack, Depth::MAX_PLY + 10> stacks;

    // Depths search
    Depth search_depth;

    // Nodes search
    uint64_t search_nodes;

    // Time & Clock & Ponder search
    uint64_t search_time;
    Timer    timer;
    bool     timer_stopped;
    bool     run_timer;
    bool     do_time_management;

    // Search parameters
    MoveList<RootEntry>                           root_moves;
    Value                                         contempt;
    bool                                          root_in_TB = false;
    bool                                          abort;
    uint64_t                                      total_nodes;
    uint64_t                                      tb_hits;
    const Depth                                   initial_depth = Depth(1);
    Depth                                         current_depth;
    Depth                                         current_max_depth;
    Move                                          current_move;
    int                                           current_move_number;
    std::array<MoveVariation, Depth::MAX_PLY + 1> pv;

    void check_stop_conditions();

    void update_search(int ply);

    void save_pv(const Move move, const MoveVariation& src, MoveVariation& dest);

    Value pv_search(Depth  depth,
                    Stack* ss,
                    Value  alpha,
                    Value  beta,
                    int    ply,
                    int    move_number);

    Value search_root(Depth depth, Stack* ss, Value alpha, Value beta);

    Value search(Depth depth, Stack* ss, Value alpha, Value beta, int ply);

    Value quiescent(Stack* ss, Value alpha, Value beta, int ply);
};

inline uint64_t Search::get_total_nodes()
{
    return total_nodes;
}

inline void
    Search::save_pv(const Move move, const MoveVariation& src, MoveVariation& dest)
{
    dest.moves[0] = move;
    for (int i = 0; i < src.size; i++)
    {
        dest.moves[i + 1] = src.moves[i];
    }
    dest.size = src.size + 1;
}

}  // namespace goldfish
