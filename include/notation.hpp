#pragma once

#include "position.hpp"

#include <array>

namespace goldfish
{
class Notation
{
public:
    static const std::string STANDARDPOSITION;

    static Position to_position(const std::string& fen);

    static std::string from_position(const Position& position);

    static Color to_color(char notation);

    static char from_color(Color color);

    static PieceType to_piece_type(char notation);

    static char from_piece_type(PieceType piecetype);

    static Piece to_piece(char notation);

    static char from_piece(Piece piece);

    static CastlingType to_castling_type(char notation);

    static char from_castling_type(CastlingType castlingtype);

    static Castling to_castling(char notation);

    static char from_castling(Castling castling);

    static File to_file(char notation);

    static char from_file(File file);

    static Rank to_rank(char notation);

    static char from_rank(Rank rank);

    static Square to_square(const std::string& notation);

    static std::string from_square(Square square);

    static std::string from_move(Move m);

    Notation() = delete;

    ~Notation() = delete;

private:
    static const char WHITE_NOTATION = 'w';
    static const char BLACK_NOTATION = 'b';

    static const char PAWN_NOTATION   = 'P';
    static const char KNIGHT_NOTATION = 'N';
    static const char BISHOP_NOTATION = 'B';
    static const char ROOK_NOTATION   = 'R';
    static const char QUEEN_NOTATION  = 'Q';
    static const char KING_NOTATION   = 'K';

    static const char KING_SIDE_NOTATION  = 'K';
    static const char QUEEN_SIDE_NOTATION = 'Q';

    static const char A_NOTATION = 'a';
    static const char B_NOTATION = 'b';
    static const char C_NOTATION = 'c';
    static const char D_NOTATION = 'd';
    static const char E_NOTATION = 'e';
    static const char F_NOTATION = 'f';
    static const char G_NOTATION = 'g';
    static const char H_NOTATION = 'h';

    static const char R1_NOTATION = '1';
    static const char R2_NOTATION = '2';
    static const char R3_NOTATION = '3';
    static const char R4_NOTATION = '4';
    static const char R5_NOTATION = '5';
    static const char R6_NOTATION = '6';
    static const char R7_NOTATION = '7';
    static const char R8_NOTATION = '8';

    static Color color_of(char notation);

    static char transform(char notation, Color color);
};

}  // namespace goldfish
