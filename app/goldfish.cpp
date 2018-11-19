#include "goldfish.hpp"
#include <iostream>

int main() {
    std::unique_ptr<goldfish::Goldfish> engine(new goldfish::Goldfish());
    engine->run();
}
