#pragma once

#include "movegenerator.hpp"

namespace goldfish
{
class Perft
{
public:
    void run();

private:
    static const int MAX_DEPTH = 6;

    std::array<MoveGenerator, MAX_DEPTH> move_generators;

    uint64_t mini_max(int depth, Position& position, int ply);
};

}  // namespace goldfish
