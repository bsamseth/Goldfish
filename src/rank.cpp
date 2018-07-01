#include "rank.hpp"

namespace goldfish {

const std::array<int, Rank::VALUES_SIZE> Rank::values = {
        R1, R2, R3, R4, R5, R6, R7, R8
};

bool Rank::is_valid(int rank) {
    switch (rank) {
        case R1:
        case R2:
        case R3:
        case R4:
        case R5:
        case R6:
        case R7:
        case R8:
            return true;
        default:
            return false;
    }
}

}
