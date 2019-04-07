#include "goldfish.hpp"

#include <iostream>

int main()
{
    goldfish::Goldfish engine;
    engine.receive_initialize();
    engine.receive_bench();
    engine.receive_quit();
}
