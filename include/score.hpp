#pragma once

#include "operations.hpp"

namespace goldfish {

enum Score : int {
    SCORE_ZERO = 0
};

ENABLE_FULL_OPERATORS_ON(Score);

}