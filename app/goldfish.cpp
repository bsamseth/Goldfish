/*
 * Copyright (C) 2013-2016 Phokham Nonava
 *
 * Use of this source code is governed by the MIT license that can be
 * found in the LICENSE file.
 */

#include "goldfish.hpp"
#include "perft.hpp"

#include <iostream>

int print_usage() {
    std::cout << "Usage: pulse [perft]" << std::endl;
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::unique_ptr<goldfish::Goldfish> goldfish(new goldfish::Goldfish());
        goldfish->run();
    } else if (argc == 2) {
        std::string token(argv[1]);
        if (token == "perft") {
            std::unique_ptr<goldfish::Perft> perft(new goldfish::Perft());
            perft->run();
        } else {
            return print_usage();
        }
    } else {
        return print_usage();
    }
}
