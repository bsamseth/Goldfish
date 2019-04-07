#pragma once

#include "notation.hpp"
#include "protocol.hpp"
#include "search.hpp"

namespace goldfish
{
class Goldfish : public Protocol
{
public:
    Goldfish() : search(*this) {}

    void run();

    void send_best_move(Move best_move, Move ponder_move) final;

    void send_status(int      current_depth,
                     int      current_max_depth,
                     uint64_t total_nodes,
                     uint64_t tb_hits,
                     Move     current_move,
                     int      current_move_number) final;

    void send_status(bool     force,
                     int      current_depth,
                     int      current_max_depth,
                     uint64_t total_nodes,
                     uint64_t tb_hits,
                     Move     current_move,
                     int      current_move_number) final;

    void send_move(const RootEntry& entry,
                   int              current_depth,
                   int              current_max_depth,
                   uint64_t         total_nodes,
                   uint64_t         tb_hits) final;

private:
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point status_start_time;

    Search   search;
    Position current_position = Notation::to_position(Notation::STANDARDPOSITION);

    void receive_ready();

    void receive_new_game();

    void receive_position(std::istringstream& input);

    void receive_go(std::istringstream& input);

    void receive_ponder_hit();

    void receive_stop();

    void receive_setoption(std::istringstream& input);

public:
    void receive_initialize();

    void receive_quit();

    void receive_bench();
};

}  // namespace goldfish
