#pragma once

#include "bitboard.hpp"
#include "castling.hpp"
#include "color.hpp"
#include "depth.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "piecetype.hpp"
#include "square.hpp"
#include "value.hpp"

#include <iostream>
#include <random>

namespace goldfish
{
class Position
{
public:
    std::array<Piece, Squares::VALUES_LENGTH> board;

    std::array<std::array<uint64_t, PieceTypes::VALUES_SIZE>, Colors::VALUES_SIZE>
        pieces = {};

    std::array<Value, Colors::VALUES_SIZE> material = {};

    Castling castling_rights  = Castling::NO_CASTLING;
    Square   enpassant_square = Square::NO_SQUARE;
    Color    active_color     = Color::WHITE;
    int      halfmove_clock   = 0;

    uint64_t zobrist_key = 0;

    Position();

    Position(const Position& position);

    Position& operator=(const Position& position);

    bool operator==(const Position& position) const;

    bool operator!=(const Position& position) const;

    void set_active_color(Color active_color);

    void set_castling_right(Castling castling);

    void set_enpassant_square(Square enpassant_square);

    void set_halfmove_clock(int halfmove_clock);

    int get_fullmove_number() const;

    void set_fullmove_number(int fullmove_number);

    bool is_repetition();

    bool has_insufficient_material();

    void put(Piece piece, Square square);

    Piece remove(Square square);

    void make_move(Move move);

    void make_null_move();

    void undo_null_move();

    void undo_move(Move move);

    bool is_check() const;

    bool is_check(Color color) const;

    bool is_attacked(Square target_square, Color attacker_color) const;

    bool last_move_was_null_move();

    constexpr uint64_t get_pieces(Color c, PieceType pt) const { return pieces[c][pt]; }

    constexpr uint64_t get_pieces(Color c) const
    {
        return get_pieces(c, PieceType::PAWN) | get_pieces(c, PieceType::KNIGHT)
               | get_pieces(c, PieceType::BISHOP) | get_pieces(c, PieceType::ROOK)
               | get_pieces(c, PieceType::QUEEN) | get_pieces(c, PieceType::KING);
    }

    constexpr uint64_t get_pieces(PieceType pt) const
    {
        return get_pieces(Color::WHITE, pt) | get_pieces(Color::BLACK, pt);
    }

    template <Color Us>
    constexpr uint64_t promoting_pawns() const
    {
        constexpr uint64_t Rank8BB
            = Us == Color::WHITE ? Bitboard::Rank8BB : Bitboard::Rank1BB;
        constexpr uint64_t Rank7BB = Bitboard::shift < Us == Color::WHITE
                                         ? Direction::SOUTH
                                         : Direction::NORTH > (Rank8BB);
        const uint64_t rank8_us    = Rank8BB & get_pieces(Us);
        const uint64_t rank8_them  = Rank8BB & get_pieces(~Us);
        const uint64_t rank8_occ   = rank8_us | rank8_them;
        const uint64_t rank7_pawns = Rank7BB & get_pieces(Us, PieceType::PAWN);

        const uint64_t push_squares = ~Bitboard::shift < Us == Color::WHITE
                                          ? Direction::SOUTH
                                          : Direction::NORTH > (rank8_occ);
        const uint64_t capt_squares = Bitboard::pawn_attacks_bb<~Us>(rank8_them);

        return rank7_pawns & (push_squares | capt_squares);
    }

    constexpr uint64_t promoting_pawns(Color us) const
    {
        return us == Color::WHITE ? promoting_pawns<Color::WHITE>()
                                  : promoting_pawns<Color::BLACK>();
    }

private:
    class Zobrist
    {
    public:
        std::array<std::array<uint64_t, Squares::VALUES_LENGTH>, Pieces::VALUES_SIZE>
                                                       board;
        std::array<uint64_t, Castlings::VALUES_LENGTH> castling_rights;
        std::array<uint64_t, Squares::VALUES_LENGTH>   enpassant_square;
        uint64_t                                       active_color;

        static Zobrist& instance();

    private:
        std::independent_bits_engine<std::mt19937, 8, uint64_t> generator;

        Zobrist();

        uint64_t next();
    };

    class State
    {
    public:
        uint64_t zobrist_key      = 0;
        Castling castling_rights  = Castling::NO_CASTLING;
        Square   enpassant_square = Square::NO_SQUARE;
        int      halfmove_clock   = 0;
    };

    static const int MAX_MOVES = Depth::MAX_PLY + 1024;

    int halfmove_number = 2;

    // We will save some position parameters in a State before making a move.
    // Later we will restore them before undoing a move.
    std::array<State, MAX_MOVES> states;
    int                          states_size = 0;

    std::array<Move, MAX_MOVES> moves;
    int                         move_count = 0;

    Zobrist& zobrist;

    void clear_castling(Square square);

    bool is_attacked(Square                        target_square,
                     Piece                         attacker_piece,
                     const std::vector<Direction>& directions) const;

    bool is_attacked(Square                        target_square,
                     Piece                         attacker_piece,
                     Piece                         queen_piece,
                     const std::vector<Direction>& directions) const;
};

}  // namespace goldfish
