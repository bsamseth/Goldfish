#include "goldfish.hpp"
#include <iostream>

int main() {
    std::unique_ptr<goldfish::Goldfish> engine(new goldfish::Goldfish());
    engine->receive_initialize();
    engine->receive_bench();
    engine->receive_quit();
}
