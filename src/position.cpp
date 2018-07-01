#include <algorithm>
#include <sstream>

#include "position.hpp"
#include "move.hpp"

namespace goldfish {

// Initialize the zobrist keys
Position::Zobrist::Zobrist() {
    for (auto piece : Piece::values) {
        for (int i = 0; i < Square::VALUES_LENGTH; i++) {
            board[piece][i] = next();
        }
    }

    castling_rights[Castling::WHITE_KING_SIDE] = next();
    castling_rights[Castling::WHITE_QUEEN_SIDE] = next();
    castling_rights[Castling::BLACK_KING_SIDE] = next();
    castling_rights[Castling::BLACK_QUEEN_SIDE] = next();
    castling_rights[Castling::WHITE_KING_SIDE | Castling::WHITE_QUEEN_SIDE] =
            castling_rights[Castling::WHITE_KING_SIDE] ^ castling_rights[Castling::WHITE_QUEEN_SIDE];
    castling_rights[Castling::BLACK_KING_SIDE | Castling::BLACK_QUEEN_SIDE] =
            castling_rights[Castling::BLACK_KING_SIDE] ^ castling_rights[Castling::BLACK_QUEEN_SIDE];

    for (int i = 0; i < Square::VALUES_LENGTH; i++) {
        enpassant_square[i] = next();
    }

    active_color = next();
}

Position::Zobrist &Position::Zobrist::instance() {
    static Zobrist *instance = new Zobrist();
    return *instance;
}

uint64_t Position::Zobrist::next() {
    std::array<uint64_t, 16> bytes;
    for (int i = 0; i < 16; i++) {
        bytes[i] = generator();
    }

    uint64_t hash = 0;
    for (int i = 0; i < 16; i++) {
        hash ^= bytes[i] << ((i * 8) % 64);
    }

    return hash;
}

Position::Position()
        : zobrist(Zobrist::instance()) {
    board.fill(+Piece::NO_PIECE);
}

Position::Position(const Position &position)
        : Position() {
    this->board = position.board;
    this->pieces = position.pieces;
    this->material = position.material;
    this->castling_rights = position.castling_rights;
    this->enpassant_square = position.enpassant_square;
    this->active_color = position.active_color;
    this->halfmove_clock = position.halfmove_clock;
    this->zobrist_key = position.zobrist_key;
    this->halfmove_number = position.halfmove_number;
    this->states_size = 0;
}

Position &Position::operator=(const Position &position) {
    this->board = position.board;
    this->pieces = position.pieces;
    this->material = position.material;
    this->castling_rights = position.castling_rights;
    this->enpassant_square = position.enpassant_square;
    this->active_color = position.active_color;
    this->halfmove_clock = position.halfmove_clock;
    this->zobrist_key = position.zobrist_key;
    this->halfmove_number = position.halfmove_number;
    this->states_size = 0;

    return *this;
}

bool Position::operator==(const Position &position) const {
    return this->board == position.board
           && this->pieces == position.pieces
           && this->material == position.material
           && this->castling_rights == position.castling_rights
           && this->enpassant_square == position.enpassant_square
           && this->active_color == position.active_color
           && this->halfmove_clock == position.halfmove_clock
           && this->zobrist_key == position.zobrist_key
           && this->halfmove_number == position.halfmove_number;
}

bool Position::operator!=(const Position &position) const {
    return !(*this == position);
}

void Position::set_active_color(int active_color) {
    if (this->active_color != active_color) {
        this->active_color = active_color;
        zobrist_key ^= zobrist.active_color;
    }
}

void Position::set_castling_right(int castling) {
    if ((castling_rights & castling) == Castling::NO_CASTLING) {
        castling_rights |= castling;
        zobrist_key ^= zobrist.castling_rights[castling];
    }
}

void Position::set_enpassant_square(int enpassant_square) {
    if (this->enpassant_square != Square::NO_SQUARE) {
        zobrist_key ^= zobrist.enpassant_square[this->enpassant_square];
    }
    if (enpassant_square != Square::NO_SQUARE) {
        zobrist_key ^= zobrist.enpassant_square[enpassant_square];
    }
    this->enpassant_square = enpassant_square;
}

void Position::set_halfmove_clock(int halfmove_clock) {
    this->halfmove_clock = halfmove_clock;
}

int Position::get_fullmove_number() const {
    return halfmove_number / 2;
}

void Position::set_fullmove_number(int fullmove_number) {
    halfmove_number = fullmove_number * 2;
    if (active_color == Color::BLACK) {
        halfmove_number++;
    }
}

bool Position::is_repetition() {
    // Search back until the last halfmove_clock reset
    int j = std::max(0, states_size - halfmove_clock);
    for (int i = states_size - 2; i >= j; i -= 2) {
        if (zobrist_key == states[i].zobrist_key) {
            return true;
        }
    }

    return false;
}

bool Position::has_insufficient_material() {
    // If there is only one minor left, we are unable to checkmate
    return Bitboard::size(pieces[Color::WHITE][PieceType::PAWN]) == 0
           && Bitboard::size(pieces[Color::BLACK][PieceType::PAWN]) == 0
           && Bitboard::size(pieces[Color::WHITE][PieceType::ROOK]) == 0
           && Bitboard::size(pieces[Color::BLACK][PieceType::ROOK]) == 0
           && Bitboard::size(pieces[Color::WHITE][PieceType::QUEEN]) == 0
           && Bitboard::size(pieces[Color::BLACK][PieceType::QUEEN]) == 0
           && (Bitboard::size(pieces[Color::WHITE][PieceType::KNIGHT]) +
               Bitboard::size(pieces[Color::WHITE][PieceType::BISHOP]) <= 1)
           && (Bitboard::size(pieces[Color::BLACK][PieceType::KNIGHT]) +
               Bitboard::size(pieces[Color::BLACK][PieceType::BISHOP]) <= 1);
}

void Position::put(int piece, int square) {
    int piecetype = Piece::get_type(piece);
    int color = Piece::get_color(piece);

    board[square] = piece;
    pieces[color][piecetype] = Bitboard::add(square, pieces[color][piecetype]);
    material[color] += PieceType::get_value(piecetype);

    zobrist_key ^= zobrist.board[piece][square];
}

int Position::remove(int square) {
    int piece = board[square];

    int piecetype = Piece::get_type(piece);
    int color = Piece::get_color(piece);

    board[square] = Piece::NO_PIECE;
    pieces[color][piecetype] = Bitboard::remove(square, pieces[color][piecetype]);
    material[color] -= PieceType::get_value(piecetype);

    zobrist_key ^= zobrist.board[piece][square];

    return piece;
}

void Position::make_null_move() {
    State &entry = states[states_size];
    entry.zobrist_key = zobrist_key;
    entry.castling_rights = castling_rights;
    entry.enpassant_square = enpassant_square;
    entry.halfmove_clock = halfmove_clock;

    states_size++;

    // Remove enpassant if set.
    if (enpassant_square != Square::NO_SQUARE) {
        zobrist_key ^= zobrist.enpassant_square[enpassant_square];
        enpassant_square = Square::NO_SQUARE;
    }

    // Change side to move.
    zobrist_key ^= zobrist.active_color;
    active_color = Color::swap_color(active_color);

    // Update halfmove_clock
    halfmove_clock++;

    // Update full_move_number
    halfmove_number++;
}

void Position::undo_null_move() {
    states_size--;
    State &entry = states[states_size];

    zobrist_key = entry.zobrist_key;
    enpassant_square = entry.enpassant_square;

    active_color = Color::swap_color(active_color);

    halfmove_clock--;
    halfmove_number--;
}

void Position::make_move(int move) {
    // Save state
    State &entry = states[states_size];
    entry.zobrist_key = zobrist_key;
    entry.castling_rights = castling_rights;
    entry.enpassant_square = enpassant_square;
    entry.halfmove_clock = halfmove_clock;

    states_size++;

    // Get variables
    int type = Move::get_type(move);
    int origin_square = Move::get_origin_square(move);
    int target_square = Move::get_target_square(move);
    int origin_piece = Move::get_origin_piece(move);
    int origin_color = Piece::get_color(origin_piece);
    int target_piece = Move::get_target_piece(move);

    // Remove target piece and update castling rights
    if (target_piece != Piece::NO_PIECE) {
        int capture_square = target_square;
        if (type == MoveType::EN_PASSANT) {
            capture_square += (origin_color == Color::WHITE ? Square::S : Square::N);
        }
        remove(capture_square);

        clear_castling(capture_square);
    }

    // Move piece
    remove(origin_square);
    if (type == MoveType::PAWN_PROMOTION) {
        put(Piece::value_of(origin_color, Move::get_promotion(move)), target_square);
    } else {
        put(origin_piece, target_square);
    }

    // Move rook and update castling rights
    if (type == MoveType::CASTLING) {
        int rook_origin_square;
        int rook_target_square;
        switch (target_square) {
            case Square::G1:
                rook_origin_square = Square::H1;
                rook_target_square = Square::F1;
                break;
            case Square::C1:
                rook_origin_square = Square::A1;
                rook_target_square = Square::D1;
                break;
            case Square::G8:
                rook_origin_square = Square::H8;
                rook_target_square = Square::F8;
                break;
            case Square::C8:
                rook_origin_square = Square::A8;
                rook_target_square = Square::D8;
                break;
            default:
                throw std::exception();
        }

        int rook_piece = remove(rook_origin_square);
        put(rook_piece, rook_target_square);
    }

    // Update castling
    clear_castling(origin_square);

    // Update enpassant_square
    if (enpassant_square != Square::NO_SQUARE) {
        zobrist_key ^= zobrist.enpassant_square[enpassant_square];
    }
    if (type == MoveType::PAWN_DOUBLE) {
        enpassant_square = target_square + (origin_color == Color::WHITE ? Square::S : Square::N);
        zobrist_key ^= zobrist.enpassant_square[enpassant_square];
    } else {
        enpassant_square = Square::NO_SQUARE;
    }

    // Update active_color
    active_color = Color::swap_color(active_color);
    zobrist_key ^= zobrist.active_color;

    // Update halfmove_clock
    if (Piece::get_type(origin_piece) == PieceType::PAWN || target_piece != Piece::NO_PIECE) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    // Update full_move_number
    halfmove_number++;
}

void Position::undo_move(int move) {
    // Get variables
    int type = Move::get_type(move);
    int origin_square = Move::get_origin_square(move);
    int target_square = Move::get_target_square(move);
    int origin_piece = Move::get_origin_piece(move);
    int origin_color = Piece::get_color(origin_piece);
    int target_piece = Move::get_target_piece(move);

    // Update full_move_number
    halfmove_number--;

    // Update active_color
    active_color = Color::swap_color(active_color);

    // Undo move rook
    if (type == MoveType::CASTLING) {
        int rook_origin_square;
        int rook_target_square;
        switch (target_square) {
            case Square::G1:
                rook_origin_square = Square::H1;
                rook_target_square = Square::F1;
                break;
            case Square::C1:
                rook_origin_square = Square::A1;
                rook_target_square = Square::D1;
                break;
            case Square::G8:
                rook_origin_square = Square::H8;
                rook_target_square = Square::F8;
                break;
            case Square::C8:
                rook_origin_square = Square::A8;
                rook_target_square = Square::D8;
                break;
            default:
                throw std::exception();
        }

        int rook_piece = remove(rook_target_square);
        put(rook_piece, rook_origin_square);
    }

    // Undo move piece
    remove(target_square);
    put(origin_piece, origin_square);

    // Restore target piece
    if (target_piece != Piece::NO_PIECE) {
        int capture_square = target_square;
        if (type == MoveType::EN_PASSANT) {
            capture_square += (origin_color == Color::WHITE ? Square::S : Square::N);
        }
        put(target_piece, capture_square);
    }

    // Restore state
    states_size--;

    State &entry = states[states_size];
    halfmove_clock = entry.halfmove_clock;
    enpassant_square = entry.enpassant_square;
    castling_rights = entry.castling_rights;
    zobrist_key = entry.zobrist_key;
}

void Position::clear_castling(int square) {
    int new_castling_rights = castling_rights;

    switch (square) {
        case Square::A1:
            new_castling_rights &= ~Castling::WHITE_QUEEN_SIDE;
            break;
        case Square::A8:
            new_castling_rights &= ~Castling::BLACK_QUEEN_SIDE;
            break;
        case Square::H1:
            new_castling_rights &= ~Castling::WHITE_KING_SIDE;
            break;
        case Square::H8:
            new_castling_rights &= ~Castling::BLACK_KING_SIDE;
            break;
        case Square::E1:
            new_castling_rights &= ~(Castling::WHITE_KING_SIDE | Castling::WHITE_QUEEN_SIDE);
            break;
        case Square::E8:
            new_castling_rights &= ~(Castling::BLACK_KING_SIDE | Castling::BLACK_QUEEN_SIDE);
            break;
        default:
            return;
    }

    if (new_castling_rights != castling_rights) {
        castling_rights = new_castling_rights;
        zobrist_key ^= zobrist.castling_rights[new_castling_rights ^ castling_rights];
    }
}

bool Position::is_check() {
    // Check whether our king is attacked by any opponent piece
    return is_attacked(Bitboard::next(pieces[active_color][PieceType::KING]), Color::swap_color(active_color));
}

bool Position::is_check(int color) {
    // Check whether the king for color is attacked by any opponent piece
    return is_attacked(Bitboard::next(pieces[color][PieceType::KING]), Color::swap_color(color));
}

/**
 * Returns whether the target_square is attacked by any piece from the
 * attacker_color. We will backtrack from the target_square to find the piece.
 *
 * @param target_square  the target Square.
 * @param attacker_color the attacker Color.
 * @return whether the target_square is attacked.
 */
bool Position::is_attacked(int target_square, int attacker_color) {
    // Pawn attacks
    int pawn_piece = Piece::value_of(attacker_color, PieceType::PAWN);
    for (unsigned int i = 1; i < Square::pawn_directions[attacker_color].size(); i++) {
        int attacker_square = target_square - Square::pawn_directions[attacker_color][i];
        if (Square::is_valid(attacker_square)) {
            int attacker_pawn = board[attacker_square];

            if (attacker_pawn == pawn_piece) {
                return true;
            }
        }
    }

    return is_attacked(target_square,
                       Piece::value_of(attacker_color, PieceType::KNIGHT),
                       Square::knight_directions)

           // The queen moves like a bishop, so check both piece types
           || is_attacked(target_square,
                          Piece::value_of(attacker_color, PieceType::BISHOP),
                          Piece::value_of(attacker_color, PieceType::QUEEN),
                          Square::bishop_directions)

           // The queen moves like a rook, so check both piece types
           || is_attacked(target_square,
                          Piece::value_of(attacker_color, PieceType::ROOK),
                          Piece::value_of(attacker_color, PieceType::QUEEN),
                          Square::rook_directions)

           || is_attacked(target_square,
                          Piece::value_of(attacker_color, PieceType::KING),
                          Square::king_directions);
}

/**
 * Returns whether the target_square is attacked by a non-sliding piece.
 */
bool Position::is_attacked(int target_square, int attacker_piece, const std::vector<int> &directions) {
    for (auto direction : directions) {
        int attacker_square = target_square + direction;

        if (Square::is_valid(attacker_square) && board[attacker_square] == attacker_piece) {
            return true;
        }
    }

    return false;
}

/**
 * Returns whether the target_square is attacked by a sliding piece.
 */
bool Position::is_attacked(int target_square, int attacker_piece, int queen_piece, const std::vector<int> &directions) {
    for (auto direction : directions) {
        int attacker_square = target_square + direction;

        while (Square::is_valid(attacker_square)) {
            int piece = board[attacker_square];

            if (Piece::is_valid(piece)) {
                if (piece == attacker_piece || piece == queen_piece) {
                    return true;
                }

                break;
            } else {
                attacker_square += direction;
            }
        }
    }

    return false;
}

}