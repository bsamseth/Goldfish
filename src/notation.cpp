#include <sstream>

#include "notation.hpp"
#include "file.hpp"
#include "rank.hpp"
#include "castlingtype.hpp"


namespace goldfish {

const std::string Notation::STANDARDPOSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Position Notation::to_position(const std::string &fen) {
    Position position;

    // Clean and split into tokens
    std::vector<std::string> tokens;
    std::stringstream ss(fen);
    std::string token;
    while (std::getline(ss, token, ' ')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    // halfmove clock and fullmove number are optional
    if (tokens.size() < 4 || tokens.size() > 6) {
        throw std::exception();
    }

    unsigned int tokens_index = 0;

    // Parse pieces
    token = tokens[tokens_index++];
    int file = File::A;
    int rank = Rank::R8;

    for (auto character : token) {
        int piece = to_piece(character);
        if (piece != Piece::NO_PIECE) {
            if (!File::is_valid(file) || !Rank::is_valid(rank)) {
                throw std::invalid_argument("Illegal file or rank");
            }

            position.put(piece, Square::value_of(file, rank));

            if (file == File::H) {
                file = File::NO_FILE;
            } else {
                file++;
            }
        } else if (character == '/') {
            if (file != File::NO_FILE || rank == Rank::R1) {
                throw std::invalid_argument("Illegal file or rank");
            }

            file = File::A;
            rank--;
        } else {
            std::string s = {character};
            int empty_squares = std::stoi(s);
            if (empty_squares < 1 || 8 < empty_squares) {
                throw std::invalid_argument("Illegal number of empty squares");
            }

            file += empty_squares - 1;
            if (!File::is_valid(file)) {
                throw std::invalid_argument("Illegal number of empty squares");
            }

            if (file == File::H) {
                file = File::NO_FILE;
            } else {
                file++;
            }
        }
    }

    // Parse active color
    token = tokens[tokens_index++];

    if (token.length() != 1) {
        throw std::exception();
    }

    int active_color = to_color(token[0]);
    if (active_color == Color::NO_COLOR) {
        throw std::exception();
    }
    position.set_active_color(active_color);

    // Parse castling rights
    token = tokens[tokens_index++];

    if (token != "-") {
        for (auto character : token) {
            int castling_file;
            int king_file;
            int castling = to_castling(character);
            if (castling == Castling::NO_CASTLING) {
                castling_file = to_file(character);
                if (castling_file == File::NO_FILE) {
                    throw std::exception();
                }

                int color = color_of(character);

                if (position.pieces[color][PieceType::KING] == 0) {
                    throw std::exception();
                }

                king_file = Square::get_file(Bitboard::next(position.pieces[color][PieceType::KING]));
                if (castling_file > king_file) {
                    castling = Castling::value_of(color, CastlingType::KING_SIDE);
                } else {
                    castling = Castling::value_of(color, CastlingType::QUEEN_SIDE);
                }
            } else if (Castling::get_type(castling) == CastlingType::KING_SIDE) {
                castling_file = File::H;
                king_file = File::E;
            } else {
                castling_file = File::A;
                king_file = File::E;
            }

            position.set_castling_right(castling);
        }
    }

    // Parse en passant square
    token = tokens[tokens_index++];

    if (token != "-") {
        if (token.length() != 2) {
            throw std::exception();
        }

        int enpassant_file = to_file(token[0]);
        int enpassant_rank = to_rank(token[1]);
        if (!(active_color == Color::BLACK && enpassant_rank == Rank::R3)
            && !(active_color == Color::WHITE && enpassant_rank == Rank::R6)) {
            throw std::exception();
        }

        position.set_enpassant_square(Square::value_of(enpassant_file, enpassant_rank));
    }

    // Parse halfmove clock
    if (tokens.size() >= 5) {
        token = tokens[tokens_index++];

        int number = std::stoi(token);
        if (number < 0) {
            throw std::exception();
        }

        position.set_halfmove_clock(number);
    }

    // Parse fullmove number
    if (tokens.size() == 6) {
        token = tokens[tokens_index++];

        int number = std::stoi(token);
        if (number < 1) {
            throw std::exception();
        }

        position.set_fullmove_number(number);
    }

    return position;
}

std::string Notation::from_position(const Position &position) {
    std::string fen;

    // Pieces
    for (auto iter = Rank::values.rbegin(); iter != Rank::values.rend(); iter++) {
        int rank = *iter;
        unsigned int empty_squares = 0;

        for (auto file : File::values) {
            int piece = position.board[Square::value_of(file, rank)];

            if (piece == Piece::NO_PIECE) {
                empty_squares++;
            } else {
                if (empty_squares > 0) {
                    fen += std::to_string(empty_squares);
                    empty_squares = 0;
                }
                fen += from_piece(piece);
            }
        }

        if (empty_squares > 0) {
            fen += std::to_string(empty_squares);
        }

        if (rank > Rank::R1) {
            fen += '/';
        }
    }

    fen += ' ';

    // Color
    fen += from_color(position.active_color);

    fen += ' ';

    // Castling
    std::string castling_notation;
    if ((position.castling_rights & Castling::WHITE_KING_SIDE) != Castling::NO_CASTLING) {
        castling_notation += from_castling(Castling::WHITE_KING_SIDE);
    }
    if ((position.castling_rights & Castling::WHITE_QUEEN_SIDE) != Castling::NO_CASTLING) {
        castling_notation += from_castling(Castling::WHITE_QUEEN_SIDE);
    }
    if ((position.castling_rights & Castling::BLACK_KING_SIDE) != Castling::NO_CASTLING) {
        castling_notation += from_castling(Castling::BLACK_KING_SIDE);
    }
    if ((position.castling_rights & Castling::BLACK_QUEEN_SIDE) != Castling::NO_CASTLING) {
        castling_notation += from_castling(Castling::BLACK_QUEEN_SIDE);
    }
    if (castling_notation.empty()) {
        fen += '-';
    } else {
        fen += castling_notation;
    }

    fen += ' ';

    // En passant
    if (position.enpassant_square != Square::NO_SQUARE) {
        fen += from_square(position.enpassant_square);
    } else {
        fen += '-';
    }

    fen += ' ';

    // Halfmove clock
    fen += std::to_string(position.halfmove_clock);

    fen += ' ';

    // Fullmove number
    fen += std::to_string(position.get_fullmove_number());

    return fen;
}

int Notation::to_color(char notation) {
    char lowercase_notation = std::tolower(notation);
    switch (lowercase_notation) {
        case WHITE_NOTATION:
            return Color::WHITE;
        case BLACK_NOTATION:
            return Color::BLACK;
        default:
            return Color::NO_COLOR;
    }
}

char Notation::from_color(int color) {
    switch (color) {
        case Color::WHITE:
            return WHITE_NOTATION;
        case Color::BLACK:
            return BLACK_NOTATION;
        case Color::NO_COLOR:
        default:
            throw std::exception();
    }
}

int Notation::color_of(char notation) {
    if (std::islower(notation)) {
        return Color::BLACK;
    } else {
        return Color::WHITE;
    }
}

char Notation::transform(char notation, int color) {
    switch (color) {
        case Color::WHITE:
            return std::toupper(notation);
        case Color::BLACK:
            return std::tolower(notation);
        default:
            throw std::exception();
    }
}

int Notation::to_piece_type(char notation) {
    char uppercase_notation = std::toupper(notation);
    switch (uppercase_notation) {
        case PAWN_NOTATION:
            return PieceType::PAWN;
        case KNIGHT_NOTATION:
            return PieceType::KNIGHT;
        case BISHOP_NOTATION:
            return PieceType::BISHOP;
        case ROOK_NOTATION:
            return PieceType::ROOK;
        case QUEEN_NOTATION:
            return PieceType::QUEEN;
        case KING_NOTATION:
            return PieceType::KING;
        default:
            return PieceType::NO_PIECE_TYPE;
    }
}

char Notation::from_piece_type(int piecetype) {
    switch (piecetype) {
        case PieceType::PAWN:
            return PAWN_NOTATION;
        case PieceType::KNIGHT:
            return KNIGHT_NOTATION;
        case PieceType::BISHOP:
            return BISHOP_NOTATION;
        case PieceType::ROOK:
            return ROOK_NOTATION;
        case PieceType::QUEEN:
            return QUEEN_NOTATION;
        case PieceType::KING:
            return KING_NOTATION;
        case PieceType::NO_PIECE_TYPE:
        default:
            throw std::exception();
    }
}

int Notation::to_piece(char notation) {
    int color = color_of(notation);
    int piecetype = to_piece_type(notation);

    if (piecetype != PieceType::NO_PIECE_TYPE) {
        return Piece::value_of(color, piecetype);
    } else {
        return Piece::NO_PIECE;
    }
}

char Notation::from_piece(int piece) {
    return transform(from_piece_type(Piece::get_type(piece)), Piece::get_color(piece));
}

int Notation::to_castling_type(char notation) {
    char uppercase_notation = std::toupper(notation);
    switch (uppercase_notation) {
        case KING_SIDE_NOTATION:
            return CastlingType::KING_SIDE;
        case QUEEN_SIDE_NOTATION:
            return CastlingType::QUEEN_SIDE;
        default:
            return CastlingType::NO_CASTLING_TYPE;
    }
}

char Notation::from_castling_type(int castlingtype) {
    switch (castlingtype) {
        case CastlingType::KING_SIDE:
            return KING_SIDE_NOTATION;
        case CastlingType::QUEEN_SIDE:
            return QUEEN_SIDE_NOTATION;
        case CastlingType::NO_CASTLING_TYPE:
        default:
            throw std::exception();
    }
}

int Notation::to_castling(char notation) {
    int color = color_of(notation);
    int castlingtype = to_castling_type(notation);

    if (castlingtype != CastlingType::NO_CASTLING_TYPE) {
        return Castling::value_of(color, castlingtype);
    } else {
        return Castling::NO_CASTLING;
    }
}

char Notation::from_castling(int castling) {
    return transform(from_castling_type(Castling::get_type(castling)), Castling::get_color(castling));
}

int Notation::to_file(char notation) {
    char lowercase_notation = std::tolower(notation);
    switch (lowercase_notation) {
        case A_NOTATION:
            return File::A;
        case B_NOTATION:
            return File::B;
        case C_NOTATION:
            return File::C;
        case D_NOTATION:
            return File::D;
        case E_NOTATION:
            return File::E;
        case F_NOTATION:
            return File::F;
        case G_NOTATION:
            return File::G;
        case H_NOTATION:
            return File::H;
        default:
            return File::NO_FILE;
    }
}

char Notation::from_file(int file) {
    switch (file) {
        case File::A:
            return A_NOTATION;
        case File::B:
            return B_NOTATION;
        case File::C:
            return C_NOTATION;
        case File::D:
            return D_NOTATION;
        case File::E:
            return E_NOTATION;
        case File::F:
            return F_NOTATION;
        case File::G:
            return G_NOTATION;
        case File::H:
            return H_NOTATION;
        case File::NO_FILE:
        default:
            throw std::exception();
    }
}

int Notation::to_rank(char notation) {
    switch (notation) {
        case R1_NOTATION:
            return Rank::R1;
        case R2_NOTATION:
            return Rank::R2;
        case R3_NOTATION:
            return Rank::R3;
        case R4_NOTATION:
            return Rank::R4;
        case R5_NOTATION:
            return Rank::R5;
        case R6_NOTATION:
            return Rank::R6;
        case R7_NOTATION:
            return Rank::R7;
        case R8_NOTATION:
            return Rank::R8;
        default:
            return Rank::NO_RANK;
    }
}

char Notation::from_rank(int rank) {
    switch (rank) {
        case Rank::R1:
            return R1_NOTATION;
        case Rank::R2:
            return R2_NOTATION;
        case Rank::R3:
            return R3_NOTATION;
        case Rank::R4:
            return R4_NOTATION;
        case Rank::R5:
            return R5_NOTATION;
        case Rank::R6:
            return R6_NOTATION;
        case Rank::R7:
            return R7_NOTATION;
        case Rank::R8:
            return R8_NOTATION;
        case Rank::NO_RANK:
        default:
            throw std::exception();
    }
}

int Notation::to_square(const std::string &notation) {
    int file = to_file(notation[0]);
    int rank = to_rank(notation[1]);

    if (file != File::NO_FILE && rank != Rank::NO_RANK) {
        return (rank << 4) + file;
    } else {
        return Square::NO_SQUARE;
    }
}

std::string Notation::from_square(int square) {
    std::string notation;
    notation += from_file(Square::get_file(square));
    notation += from_rank(Square::get_rank(square));

    return notation;
}

}
