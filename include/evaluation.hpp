#pragma once

#include <vector>

#include "value.hpp"
#include "color.hpp"
#include "square.hpp"
#include "position.hpp"

namespace goldfish::Evaluation {

Value evaluate(const Position& position);

}
