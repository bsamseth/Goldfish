#pragma once

#include "position.hpp"


namespace goldfish::Evaluation {

constexpr int TEMPO = 1;
constexpr int MAX_WEIGHT = 100;
constexpr int material_weight = 100;
constexpr int mobility_weight = 80;

int evaluate(Position &position);

int evaluate_material(Color color, Position &position);

int evaluate_mobility(Color color, Position &position);

int evaluate_mobility(Color color, Position &position, int square, const std::vector<int> &directions);

}
