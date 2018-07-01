#pragma once

#include "bitboard.hpp"
#include "color.hpp"
#include "castling.hpp"
#include "square.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "depth.hpp"

#include <random>

namespace goldfish {

class Position {
public:
    std::array<int, Square::VALUES_LENGTH> board;

    std::array<std::array<uint64_t, PieceType::VALUES_SIZE>, Color::VALUES_SIZE> pieces = {};

    std::array<int, Color::VALUES_SIZE> material = {};

    int castling_rights = Castling::NO_CASTLING;
    int enpassant_square = Square::NO_SQUARE;
    int active_color = Color::WHITE;
    int halfmove_clock = 0;

    uint64_t zobrist_key = 0;

    Position();

    Position(const Position &position);

    Position &operator=(const Position &position);

    bool operator==(const Position &position) const;

    bool operator!=(const Position &position) const;

    void set_active_color(int active_color);

    void set_castling_right(int castling);

    void set_enpassant_square(int enpassant_square);

    void set_halfmove_clock(int halfmove_clock);

    int get_fullmove_number() const;

    void set_fullmove_number(int fullmove_number);

    bool is_repetition();

    bool has_insufficient_material();

    void put(int piece, int square);

    int remove(int square);

    void make_move(int move);

    void undo_move(int move);

    bool is_check();

    bool is_check(int color);

    bool is_attacked(int target_square, int attacker_color);

private:
    class Zobrist {
    public:
        std::array<std::array<uint64_t, Square::VALUES_LENGTH>, Piece::VALUES_SIZE> board;
        std::array<uint64_t, Castling::VALUES_LENGTH> castling_rights;
        std::array<uint64_t, Square::VALUES_LENGTH> enpassant_square;
        uint64_t active_color;

        static Zobrist &instance();

    private:
        std::independent_bits_engine<std::mt19937, 8, uint64_t> generator;

        Zobrist();

        uint64_t next();
    };

    class State {
    public:
        uint64_t zobrist_key = 0;
        int castling_rights = Castling::NO_CASTLING;
        int enpassant_square = Square::NO_SQUARE;
        int halfmove_clock = 0;
    };

    static const int MAX_MOVES = Depth::MAX_PLY + 1024;

    int halfmove_number = 2;

    // We will save some position parameters in a State before making a move.
    // Later we will restore them before undoing a move.
    std::array<State, MAX_MOVES> states;
    int states_size = 0;

    Zobrist &zobrist;

    void clear_castling(int square);

    bool is_attacked(int target_square, int attacker_piece, const std::vector<int> &directions);

    bool is_attacked(int target_square, int attacker_piece, int queen_piece, const std::vector<int> &directions);
};

}
