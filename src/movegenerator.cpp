#include "movegenerator.hpp"
#include "rank.hpp"

namespace goldfish {

MoveList<MoveEntry> &MoveGenerator::get_legal_moves(Position &position, int depth, bool is_check) {
    MoveList<MoveEntry> &legal_moves = get_moves(position, depth, is_check);

    int size = legal_moves.size;
    legal_moves.size = 0;
    for (int i = 0; i < size; i++) {
        int move = legal_moves.entries[i]->move;

        position.make_move(move);
        if (!position.is_check(~position.active_color)) {
            legal_moves.entries[legal_moves.size++]->move = move;
        }
        position.undo_move(move);
    }

    return legal_moves;
}

MoveList<MoveEntry> &MoveGenerator::get_moves(Position &position, int depth, bool is_check) {
    moves.size = 0;

    if (depth > 0) {
        // Generate main moves

        add_moves(moves, position);

        if (!is_check) {
            Square square = static_cast<Square>(Bitboard::next(position.pieces[static_cast<int>(position.active_color)][static_cast<int>(PieceType::KING)]));
            add_castling_moves(moves, square, position);
        }
    } else {
        // Generate quiescent moves

        add_moves(moves, position);

        if (!is_check) {
            int size = moves.size;
            moves.size = 0;
            for (int i = 0; i < size; i++) {
                if (Moves::get_target_piece(moves.entries[i]->move) != Piece::NO_PIECE) {
                    // Add only capturing moves
                    moves.entries[moves.size++]->move = moves.entries[i]->move;
                }
            }
        }
    }

    moves.rate_from_Mvvlva();
    moves.sort();

    return moves;
}

void MoveGenerator::add_moves(MoveList<MoveEntry> &list, Position &position) {
    int active_color = static_cast<int>(position.active_color);

    for (auto squares = position.pieces[active_color][static_cast<int>(PieceType::PAWN)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = static_cast<Square>(Bitboard::next(squares));
        add_pawn_moves(list, square, position);
    }
    for (auto squares = position.pieces[active_color][static_cast<int>(PieceType::KNIGHT)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = static_cast<Square>(Bitboard::next(squares));
        add_moves(list, square, Squares::knight_directions, position);
    }
    for (auto squares = position.pieces[active_color][static_cast<int>(PieceType::BISHOP)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = static_cast<Square>(Bitboard::next(squares));
        add_moves(list, square, Squares::bishop_directions, position);
    }
    for (auto squares = position.pieces[active_color][static_cast<int>(PieceType::ROOK)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = static_cast<Square>(Bitboard::next(squares));
        add_moves(list, square, Squares::rook_directions, position);
    }
    for (auto squares = position.pieces[active_color][static_cast<int>(PieceType::QUEEN)];
         squares != 0; squares = Bitboard::remainder(squares)) {
        Square square = static_cast<Square>(Bitboard::next(squares));
        add_moves(list, square, Squares::queen_directions, position);
    }
    Square square = static_cast<Square>(Bitboard::next(position.pieces[active_color][static_cast<int>(PieceType::KING)]));
    add_moves(list, square, Squares::king_directions, position);
}

void MoveGenerator::add_moves(MoveList<MoveEntry> &list, Square origin_square, const std::vector<int> &directions,
                              Position &position) {
    Piece origin_piece = position.board[static_cast<int>(origin_square)];
    bool sliding = PieceTypes::is_sliding(Pieces::get_type(origin_piece));
    Color opposite_color = ~Pieces::get_color(origin_piece);

    // Go through all move directions for this piece
    for (auto direction : directions) {
        Square target_square = static_cast<Square>(static_cast<int>(origin_square) + direction);

        // Check if we're still on the board
        while (Squares::is_valid(target_square)) {
            Piece target_piece = position.board[static_cast<int>(target_square)];

            if (target_piece == Piece::NO_PIECE) {
                // quiet move
                list.entries[list.size++]->move = Moves::value_of(
                        MoveType::NORMAL, origin_square, target_square, origin_piece, Piece::NO_PIECE,
                        PieceType::NO_PIECE_TYPE);

                if (!sliding) {
                    break;
                }

                target_square = static_cast<Square>(static_cast<int>(target_square) + direction);
            } else {
                if (Pieces::get_color(target_piece) == opposite_color) {
                    // capturing move
                    list.entries[list.size++]->move = Moves::value_of(
                            MoveType::NORMAL, origin_square, target_square, origin_piece, target_piece,
                            PieceType::NO_PIECE_TYPE);
                }

                break;
            }
        }
    }
}

void MoveGenerator::add_pawn_moves(MoveList<MoveEntry> &list, Square pawn_square, Position &position) {
    Piece pawn_piece = position.board[static_cast<int>(pawn_square)];
    Color pawn_color = Pieces::get_color(pawn_piece);

    // Generate only capturing moves first (i = 1)
    for (unsigned int i = 1; i < Squares::pawn_directions[static_cast<int>(pawn_color)].size(); i++) {
        int direction = Squares::pawn_directions[static_cast<int>(pawn_color)][i];

        Square target_square = static_cast<Square>(static_cast<int>(pawn_square) + direction);
        if (Squares::is_valid(target_square)) {
            Piece target_piece = position.board[static_cast<int>(target_square)];

            if (target_piece != Piece::NO_PIECE) {
                if (Pieces::get_color(target_piece) == ~pawn_color) {
                    // Capturing move

                    if ((pawn_color == Color::WHITE && Squares::get_rank(target_square) == Rank::R8)
                        || (pawn_color == Color::BLACK && Squares::get_rank(target_square) == Rank::R1)) {
                        // Pawn promotion capturing move

                        list.entries[list.size++]->move = Moves::value_of(
                                MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, target_piece,
                                PieceType::QUEEN);
                        list.entries[list.size++]->move = Moves::value_of(
                                MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, target_piece,
                                PieceType::ROOK);
                        list.entries[list.size++]->move = Moves::value_of(
                                MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, target_piece,
                                PieceType::BISHOP);
                        list.entries[list.size++]->move = Moves::value_of(
                                MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, target_piece,
                                PieceType::KNIGHT);
                    } else {
                        // Normal capturing move

                        list.entries[list.size++]->move = Moves::value_of(
                                MoveType::NORMAL, pawn_square, target_square, pawn_piece, target_piece,
                                PieceType::NO_PIECE_TYPE);
                    }
                }
            } else if (target_square == position.enpassant_square) {
                // En passant move
                Square capture_square = static_cast<Square>(static_cast<int>(target_square) + (pawn_color == Color::WHITE ? Squares::S : Squares::N));
                target_piece = position.board[static_cast<int>(capture_square)];

                list.entries[list.size++]->move = Moves::value_of(
                        MoveType::EN_PASSANT, pawn_square, target_square, pawn_piece, target_piece,
                        PieceType::NO_PIECE_TYPE);
            }
        }
    }

    // Generate non-capturing moves
    int direction = Squares::pawn_directions[static_cast<int>(pawn_color)][0];

    // Move one rank forward
    Square target_square = static_cast<Square>(static_cast<int>(pawn_square) + direction);
    if (Squares::is_valid(target_square) && position.board[static_cast<int>(target_square)] == Piece::NO_PIECE) {
        if ((pawn_color == Color::WHITE && Squares::get_rank(target_square) == Rank::R8)
            || (pawn_color == Color::BLACK && Squares::get_rank(target_square) == Rank::R1)) {
            // Pawn promotion move

            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, Piece::NO_PIECE,
                    PieceType::QUEEN);
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, Piece::NO_PIECE, PieceType::ROOK);
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, Piece::NO_PIECE,
                    PieceType::BISHOP);
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::PAWN_PROMOTION, pawn_square, target_square, pawn_piece, Piece::NO_PIECE,
                    PieceType::KNIGHT);
        } else {
            // Normal move

            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::NORMAL, pawn_square, target_square, pawn_piece, Piece::NO_PIECE,
                    PieceType::NO_PIECE_TYPE);

            // Move another rank forward
            target_square = static_cast<Square>(static_cast<int>(target_square) + direction);
            if (Squares::is_valid(target_square) && position.board[static_cast<int>(target_square)] == Piece::NO_PIECE) {
                if ((pawn_color == Color::WHITE && Squares::get_rank(target_square) == Rank::R4)
                    || (pawn_color == Color::BLACK && Squares::get_rank(target_square) == Rank::R5)) {
                    // Pawn double move

                    list.entries[list.size++]->move = Moves::value_of(
                            MoveType::PAWN_DOUBLE, pawn_square, target_square, pawn_piece, Piece::NO_PIECE,
                            PieceType::NO_PIECE_TYPE);
                }
            }
        }
    }
}

void MoveGenerator::add_castling_moves(MoveList<MoveEntry> &list, Square king_square, Position &position) {
    Piece king_piece = position.board[static_cast<int>(king_square)];

    if (Pieces::get_color(king_piece) == Color::WHITE) {
        // Do not test g1 whether it is attacked as we will test it in is_legal()
        if ((position.castling_rights & Castling::WHITE_KING_SIDE) != Castling::NO_CASTLING
            && position.board[static_cast<int>(Square::F1)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::G1)] == Piece::NO_PIECE
            && !position.is_attacked(Square::F1, Color::BLACK)) {
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::CASTLING, king_square, Square::G1, king_piece, Piece::NO_PIECE, PieceType::NO_PIECE_TYPE);
        }
        // Do not test c1 whether it is attacked as we will test it in is_legal()
        if ((position.castling_rights & Castling::WHITE_QUEEN_SIDE) != Castling::NO_CASTLING
            && position.board[static_cast<int>(Square::B1)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::C1)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::D1)] == Piece::NO_PIECE
            && !position.is_attacked(Square::D1, Color::BLACK)) {
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::CASTLING, king_square, Square::C1, king_piece, Piece::NO_PIECE, PieceType::NO_PIECE_TYPE);
        }
    } else {
        // Do not test g8 whether it is attacked as we will test it in is_legal()
        if ((position.castling_rights & Castling::BLACK_KING_SIDE) != Castling::NO_CASTLING
            && position.board[static_cast<int>(Square::F8)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::G8)] == Piece::NO_PIECE
            && !position.is_attacked(Square::F8, Color::WHITE)) {
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::CASTLING, king_square, Square::G8, king_piece, Piece::NO_PIECE, PieceType::NO_PIECE_TYPE);
        }
        // Do not test c8 whether it is attacked as we will test it in is_legal()
        if ((position.castling_rights & Castling::BLACK_QUEEN_SIDE) != Castling::NO_CASTLING
            && position.board[static_cast<int>(Square::B8)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::C8)] == Piece::NO_PIECE
            && position.board[static_cast<int>(Square::D8)] == Piece::NO_PIECE
            && !position.is_attacked(Square::D8, Color::WHITE)) {
            list.entries[list.size++]->move = Moves::value_of(
                    MoveType::CASTLING, king_square, Square::C8, king_piece, Piece::NO_PIECE, PieceType::NO_PIECE_TYPE);
        }
    }
}

}
