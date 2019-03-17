#pragma once

#include "color.hpp"
#include "position.hpp"
#include "square.hpp"
#include "value.hpp"

#include <vector>

namespace goldfish::Evaluation
{
constexpr int MAX_WEIGHT      = 100;
constexpr int material_weight = 100;
constexpr int mobility_weight = 80;

Value evaluate(const Position& position);
Value evaluate_material(Color color, const Position& position);
Value evaluate_mobility(Color color, const Position& position);
Value evaluate_mobility(const Position&               position,
                        Square                        square,
                        const std::vector<Direction>& directions);

}  // namespace goldfish::Evaluation
