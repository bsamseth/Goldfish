#include "tb.hpp"

#include <iostream>

namespace goldfish::tb
{
unsigned MAX_MAN;

bool initialize(std::string path)
{
    bool result = tb_init_impl(path.c_str());
    MAX_MAN     = TB_LARGEST;

    if (!result)
        std::cout << "info string could not load syzygy files from path: " << path
                  << '\n';
    std::cout << "info string loaded syzygy tables with largest number of pieces: "
              << MAX_MAN << '\n';

    return result;
}

}  // namespace goldfish::tb
