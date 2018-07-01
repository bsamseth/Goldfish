#include <cstdlib>
#include <cmath>

#include "value.hpp"

namespace goldfish {

bool Value::is_checkmate(int value) {
    int absvalue = std::abs(value);
    return absvalue >= CHECKMATE_THRESHOLD && absvalue <= CHECKMATE;
}

}
