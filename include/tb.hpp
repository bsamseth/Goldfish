#pragma once

#include "position.hpp"
#include "Fathom/src/tbprobe.h"
#include <string>

namespace goldfish::tb {

enum class Outcome : unsigned {
    LOSS = TB_LOSS,
    BLESSED_LOSS = TB_BLESSED_LOSS,
    DRAW = TB_DRAW,
    CURSED_WIN = TB_CURSED_WIN,
    WIN = TB_WIN,
    FAILED_PROBE = TB_RESULT_FAILED
};

// Will hold the maximum number of pieces seen in tablebase.
extern unsigned MAX_MAN;

bool initialize(std::string path);

inline Outcome probe_outcome(const Position& pos) {

    if (pos.halfmove_clock || pos.castling_rights)
        return Outcome::FAILED_PROBE;

    const Outcome outcome =
        static_cast<Outcome>(tb_probe_wdl(
            pos.get_pieces<Color::WHITE>(),
            pos.get_pieces<Color::BLACK>(),
            pos.get_pieces<PieceType::KING>(),
            pos.get_pieces<PieceType::QUEEN>(),
            pos.get_pieces<PieceType::ROOK>(),
            pos.get_pieces<PieceType::BISHOP>(),
            pos.get_pieces<PieceType::KNIGHT>(),
            pos.get_pieces<PieceType::PAWN>(),
            0,   // Half move clock
            0,   // Castling
            pos.enpassant_square == Square::NO_SQUARE ? 0 : Bitboard::to_bit_square(pos.enpassant_square),
            pos.active_color == Color::WHITE));
    return outcome;
}


}
