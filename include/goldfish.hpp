#pragma once

#include <memory>
#include "search.hpp"
#include "protocol.hpp"
#include "notation.hpp"

namespace goldfish {

class Goldfish : public Protocol {
public:
    void run();

    void send_best_move(int best_move, int ponder_move) final;

    void send_status(
            int current_depth, int current_max_depth, uint64_t total_nodes, int current_move,
            int current_move_number) final;

    void send_status(
            bool force, int current_depth, int current_max_depth, uint64_t total_nodes, int current_move,
            int current_move_number) final;

    void send_move(RootEntry entry, int current_depth, int current_max_depth, uint64_t total_nodes) final;

    static std::string from_move(int move);

private:
    std::unique_ptr<Search> search = std::make_unique<Search>(*this);
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point status_start_time;

    std::unique_ptr<Position> current_position = std::make_unique<Position>(
            Notation::to_position(Notation::STANDARDPOSITION));

    void receive_initialize();

    void receive_ready();

    void receive_new_game();

    void receive_position(std::istringstream &input);

    void receive_go(std::istringstream &input);

    void receive_ponder_hit();

    void receive_stop();

    void receive_quit();

    void receive_bench();
};

}
