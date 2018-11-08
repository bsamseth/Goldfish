#pragma once

#include "position.hpp"
#include "value.hpp"

namespace goldfish {

class Evaluation {
public:
    static const Value TEMPO = 1;

    static int material_weight;
    static int mobility_weight;

    Value evaluate(Position &position);

private:
    static const int MAX_WEIGHT = 100;

    Value evaluate_material(int color, Position &position);

    Value evaluate_mobility(int color, Position &position);

    Value evaluate_mobility(int color, Position &position, int square, const std::vector<int> &directions);
};

}
