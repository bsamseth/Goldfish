#pragma once

#include <array>
#include <string>

#include "file.hpp"
#include "rank.hpp"
#include "position.hpp"

namespace goldfish::Notation {

const std::string STANDARDPOSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Position to_position(const std::string &fen);

std::string from_position(const Position &position);

Color to_color(char notation);

char from_color(Color color);

PieceType to_piece_type(char notation);

char from_piece_type(PieceType piecetype);

Piece to_piece(char notation);

char from_piece(Piece piece);

CastlingType to_castling_type(char notation);

char from_castling_type(CastlingType castlingtype);

int to_castling(char notation);

char from_castling(int castling);

File to_file(char notation);

char from_file(File file);

Rank to_rank(char notation);

char from_rank(Rank rank);

Square to_square(const std::string &notation);

std::string from_square(Square square);

namespace Definitions {

constexpr char WHITE_NOTATION = 'w';
constexpr char BLACK_NOTATION = 'b';

constexpr char PAWN_NOTATION = 'P';
constexpr char KNIGHT_NOTATION = 'N';
constexpr char BISHOP_NOTATION = 'B';
constexpr char ROOK_NOTATION = 'R';
constexpr char QUEEN_NOTATION = 'Q';
constexpr char KING_NOTATION = 'K';

constexpr char KING_SIDE_NOTATION = 'K';
constexpr char QUEEN_SIDE_NOTATION = 'Q';

constexpr char A_NOTATION = 'a';
constexpr char B_NOTATION = 'b';
constexpr char C_NOTATION = 'c';
constexpr char D_NOTATION = 'd';
constexpr char E_NOTATION = 'e';
constexpr char F_NOTATION = 'f';
constexpr char G_NOTATION = 'g';
constexpr char H_NOTATION = 'h';

constexpr char R1_NOTATION = '1';
constexpr char R2_NOTATION = '2';
constexpr char R3_NOTATION = '3';
constexpr char R4_NOTATION = '4';
constexpr char R5_NOTATION = '5';
constexpr char R6_NOTATION = '6';
constexpr char R7_NOTATION = '7';
constexpr char R8_NOTATION = '8';

Color color_of(char notation);

char transform(char notation, Color color);
}

}

