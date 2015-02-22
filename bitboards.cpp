#include <string>
#include <sstream>
#include <cstdint>
#include <bitset>

#include "bitboards.h"
#include "types.h"


std::string Bitboards::prettyString(Bitboard b) {
    int bits [64] = {};
    std::bitset<64> bitset_of_b (b);
    
    for (int i = 63; i >= 0; i--) {
	bits[i] = bitset_of_b.test(i);
    }
    
    
    std::stringstream sstm;
    sstm << "| ";

    for (int i = 7; i >= 0; i--) {
	for (int j = 0; j < 8; j++) {
	    sstm << bits[8*i+j] << " | ";
	}
	sstm << "\n| ";
    }
    return sstm.str().substr(0,sstm.str().size() -2);
    }
