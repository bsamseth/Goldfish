#include "perft.hpp"

#include "notation.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

namespace goldfish
{
void Perft::run()
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));
    int      depth = MAX_DEPTH;

    std::cout << "Testing " << Notation::from_position(position) << " at depth "
              << depth << std::endl;

    auto     start_time = std::chrono::system_clock::now();
    uint64_t result     = mini_max(depth, position, 0);
    auto     end_time   = std::chrono::system_clock::now();

    auto duration = end_time - start_time;

    std::cout << "Nodes: ";
    std::cout << result << std::endl;
    std::cout << "Duration: ";
    std::cout << std::setfill('0') << std::setw(2)
              << std::chrono::duration_cast<std::chrono::hours>(duration).count()
              << ":";
    std::cout << std::setfill('0') << std::setw(2)
              << (std::chrono::duration_cast<std::chrono::minutes>(duration)
                  - std::chrono::duration_cast<std::chrono::minutes>(
                      std::chrono::duration_cast<std::chrono::hours>(duration)))
                     .count()
              << ":";
    std::cout << std::setfill('0') << std::setw(2)
              << (std::chrono::duration_cast<std::chrono::seconds>(duration)
                  - std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::duration_cast<std::chrono::minutes>(duration)))
                     .count()
              << ".";
    std::cout << std::setfill('0') << std::setw(2)
              << (std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                  - std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::duration_cast<std::chrono::seconds>(duration)))
                     .count();
    std::cout << std::endl;

    std::cout
        << "n/ms: "
        << result
               / std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
        << std::endl;
}

uint64_t Perft::mini_max(int depth, Position& position, int ply)
{
    if (depth == 0)
    {
        return 1;
    }

    uint64_t total_nodes = 0;

    bool                 is_check       = position.is_check();
    MoveGenerator&       move_generator = move_generators[ply];
    MoveList<MoveEntry>& moves = move_generator.get_moves(position, depth, is_check);
    for (int i = 0; i < moves.size; i++)
    {
        Move move = moves.entries[i].move;

        position.make_move(move);
        if (!position.is_check(~position.active_color))
        {
            total_nodes += mini_max(depth - 1, position, ply + 1);
        }
        position.undo_move(move);
    }

    return total_nodes;
}

}  // namespace goldfish
