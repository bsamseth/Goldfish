#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <string>
#include <iostream>

#include "types.h"
#include "move.h"
#include "position.h"

class MoveGenerator {
    protected:
    std::vector<Move> generatedMoves;
    Position* pos;

    public:
    MoveGenerator(const Position& position);
    std::vector<Move> generatedMoves();
    void generateMoves();
}

inline std::vector<Move> MoveGenerator::generatedMoves() {
    return generatedMoves;
}

#endif
