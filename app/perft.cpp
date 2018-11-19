#include "goldfish.hpp"
#include "perft.hpp"
#include <iostream>

int main() {
    std::unique_ptr<goldfish::Perft> perft(new goldfish::Perft());
    perft->run();
}
