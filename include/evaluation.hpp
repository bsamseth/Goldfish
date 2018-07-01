#pragma once

#include "position.hpp"

namespace goldfish {

class Evaluation {
public:
    static const int TEMPO = 1;

    static int material_weight;
    static int mobility_weight;

    int evaluate(Position &position);

private:
    static const int MAX_WEIGHT = 100;

    int evaluate_material(int color, Position &position);

    int evaluate_mobility(int color, Position &position);

    int evaluate_mobility(int color, Position &position, int square, const std::vector<int> &directions);
};

}
