#include "tb.hpp"

namespace goldfish::tb {

unsigned MAX_MAN;

bool initialize(std::string path) {
    bool result = tb_init_impl(path.c_str());
    MAX_MAN = TB_LARGEST;
    return result;
}

}
