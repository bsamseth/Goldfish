#include "move.hpp"
#include "notation.hpp"

#include "gtest/gtest.h"

using namespace goldfish;

TEST(positiontest, test_equals)
{
    // Standard position test
    Position position1(Notation::to_position(Notation::STANDARDPOSITION));
    Position position2(Notation::to_position(Notation::STANDARDPOSITION));

    // reflexive test
    EXPECT_EQ(position1, position1);

    // symmetric test
    EXPECT_EQ(position1, position2);
    EXPECT_EQ(position2, position1);

    // FEN test
    Position position3(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    EXPECT_EQ(position1, position3);

    Position position4(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1"));
    Position position5(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"));
    Position position6(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1"));
    Position position7(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 1 1"));
    Position position8(Notation::to_position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 2"));

    EXPECT_NE(position1, position4);
    EXPECT_NE(position1, position5);
    EXPECT_NE(position1, position6);
    EXPECT_NE(position1, position7);
    EXPECT_NE(position1, position8);
}

TEST(positiontest, test_to_string)
{
    std::string fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Position position(Notation::to_position(fen));
    EXPECT_EQ(Piece::BLACK_ROOK, position.board[Square::A8]);
    EXPECT_EQ(Piece::WHITE_ROOK, position.board[Square::A1]);
    EXPECT_EQ(Piece::BLACK_ROOK, position.board[Square::H8]);
    EXPECT_EQ(Piece::WHITE_ROOK, position.board[Square::H1]);

    EXPECT_EQ(fen, Notation::from_position(position));
}

TEST(positiontest, test_active_color)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    // Move white pawn
    Move move = Moves::value_of(MoveType::NORMAL,
                                Square::A2,
                                Square::A3,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(Color::BLACK, position.active_color);

    // Move black pawn
    move = Moves::value_of(MoveType::NORMAL,
                           Square::B7,
                           Square::B6,
                           Piece::BLACK_PAWN,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(Color::WHITE, position.active_color);
}

TEST(positiontest, test_half_moveClock)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    // Move white pawn
    Move move = Moves::value_of(MoveType::NORMAL,
                                Square::A2,
                                Square::A3,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(0, position.halfmove_clock);

    // Move black pawn
    move = Moves::value_of(MoveType::NORMAL,
                           Square::B7,
                           Square::B6,
                           Piece::BLACK_PAWN,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    // Move white knight
    move = Moves::value_of(MoveType::NORMAL,
                           Square::B1,
                           Square::C3,
                           Piece::WHITE_KNIGHT,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(1, position.halfmove_clock);
}

TEST(positiontest, test_full_moveNumber)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    // Move white pawn
    Move move = Moves::value_of(MoveType::NORMAL,
                                Square::A2,
                                Square::A3,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(1, position.get_fullmove_number());

    // Move black pawn
    move = Moves::value_of(MoveType::NORMAL,
                           Square::B7,
                           Square::B6,
                           Piece::BLACK_PAWN,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(2, position.get_fullmove_number());
}

TEST(positiontest, test_is_repetition)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    // Move white knight
    Move move = Moves::value_of(MoveType::NORMAL,
                                Square::B1,
                                Square::C3,
                                Piece::WHITE_KNIGHT,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    // Move black knight
    move = Moves::value_of(MoveType::NORMAL,
                           Square::B8,
                           Square::C6,
                           Piece::BLACK_KNIGHT,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    // Move white knight
    move = Moves::value_of(MoveType::NORMAL,
                           Square::G1,
                           Square::F3,
                           Piece::WHITE_KNIGHT,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    // Move black knight
    move = Moves::value_of(MoveType::NORMAL,
                           Square::C6,
                           Square::B8,
                           Piece::BLACK_KNIGHT,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    // Move white knight
    move = Moves::value_of(MoveType::NORMAL,
                           Square::F3,
                           Square::G1,
                           Piece::WHITE_KNIGHT,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_TRUE(position.is_repetition());
}

TEST(positiontest, test_has_insufficientMaterial)
{
    Position position(Notation::to_position("8/4k3/8/8/8/8/2K5/8 w - - 0 1"));
    EXPECT_TRUE(position.has_insufficient_material());

    position = Notation::to_position("8/4k3/8/2B5/8/8/2K5/8 b - - 0 1");
    EXPECT_TRUE(position.has_insufficient_material());

    position = Notation::to_position("8/4k3/8/2B3n1/8/8/2K5/8 b - - 0 1");
    EXPECT_TRUE(position.has_insufficient_material());
}

TEST(positiontest, test_normal_move)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    uint64_t zobrist_key = position.zobrist_key;

    Move move = Moves::value_of(MoveType::NORMAL,
                                Square::A2,
                                Square::A3,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    position.undo_move(move);

    EXPECT_EQ(Notation::STANDARDPOSITION, Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);
}

TEST(positiontest, test_pawn_doubleMove)
{
    Position position(Notation::to_position(Notation::STANDARDPOSITION));

    uint64_t zobrist_key = position.zobrist_key;

    Move move = Moves::value_of(MoveType::PAWN_DOUBLE,
                                Square::A2,
                                Square::A4,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(Square::A3, position.enpassant_square);
    position.undo_move(move);
    EXPECT_EQ(Notation::STANDARDPOSITION, Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);
}

TEST(positiontest, test_pawn_promotionMove)
{
    Position position(Notation::to_position("8/P5k1/8/8/2K5/8/8/8 w - - 0 1"));
    uint64_t zobrist_key = position.zobrist_key;

    Move move = Moves::value_of(MoveType::PAWN_PROMOTION,
                                Square::A7,
                                Square::A8,
                                Piece::WHITE_PAWN,
                                Piece::NO_PIECE,
                                PieceType::QUEEN);
    position.make_move(move);

    EXPECT_EQ(Piece::WHITE_QUEEN, position.board[Square::A8]);

    position.undo_move(move);

    EXPECT_EQ("8/P5k1/8/8/2K5/8/8/8 w - - 0 1", Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);
}

TEST(positiontest, test_enpassantMove)
{
    Position position(Notation::to_position("5k2/8/8/8/3Pp3/8/8/3K4 b - d3 0 1"));
    uint64_t zobrist_key = position.zobrist_key;

    // Make en passant move
    Move move = Moves::value_of(MoveType::EN_PASSANT,
                                Square::E4,
                                Square::D3,
                                Piece::BLACK_PAWN,
                                Piece::WHITE_PAWN,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    EXPECT_EQ(Piece::NO_PIECE, position.board[Square::D4]);
    EXPECT_EQ(Piece::BLACK_PAWN, position.board[Square::D3]);
    EXPECT_EQ(Square::NO_SQUARE, position.enpassant_square);

    position.undo_move(move);

    EXPECT_EQ("5k2/8/8/8/3Pp3/8/8/3K4 b - d3 0 1", Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);
}

TEST(positiontest, test_castling_move)
{
    Position position(Notation::to_position("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"));
    uint64_t zobrist_key = position.zobrist_key;

    Move move = Moves::value_of(MoveType::CASTLING,
                                Square::E1,
                                Square::C1,
                                Piece::WHITE_KING,
                                Piece::NO_PIECE,
                                PieceType::NO_PIECE_TYPE);
    position.make_move(move);
    EXPECT_EQ(Castling::NO_CASTLING,
              position.castling_rights & Castling::WHITE_QUEEN_SIDE);
    position.undo_move(move);
    EXPECT_EQ("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);

    position    = Notation::to_position("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    zobrist_key = position.zobrist_key;

    move = Moves::value_of(MoveType::CASTLING,
                           Square::E1,
                           Square::G1,
                           Piece::WHITE_KING,
                           Piece::NO_PIECE,
                           PieceType::NO_PIECE_TYPE);
    position.make_move(move);

    EXPECT_EQ(Castling::NO_CASTLING,
              position.castling_rights & Castling::WHITE_KING_SIDE);

    position.undo_move(move);

    EXPECT_EQ("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              Notation::from_position(position));
    EXPECT_EQ(zobrist_key, position.zobrist_key);
}

TEST(positiontest, test_null_move)
{
    // Test with position that has enpassant.
    Position initial_position(
        Notation::to_position("5k2/8/8/8/3Pp3/8/8/3K4 b - d3 0 1"));
    Position copy = initial_position;
    copy.make_null_move();

    EXPECT_EQ(~initial_position.active_color, copy.active_color);
    EXPECT_EQ(copy.board, initial_position.board);
    EXPECT_EQ(copy.pieces, initial_position.pieces);

    copy.undo_null_move();

    EXPECT_EQ(copy, initial_position);
}

TEST(positiontest, test_bitboard_fetchers)
{
    const Position pos(Notation::to_position("8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1"));

    const U64 wking   = 1ULL << 1;
    const U64 bking   = 1ULL << 29;
    const U64 wpawn   = 1ULL << 16;
    const U64 bpawn   = 1ULL << 30;
    const U64 wknight = 1ULL << 3;

    ASSERT_EQ(wking, (pos.get_pieces(Color::WHITE, PieceType::KING)));
    ASSERT_EQ(bking, (pos.get_pieces(Color::BLACK, PieceType::KING)));
    ASSERT_EQ(wpawn, (pos.get_pieces(Color::WHITE, PieceType::PAWN)));
    ASSERT_EQ(bpawn, (pos.get_pieces(Color::BLACK, PieceType::PAWN)));
    ASSERT_EQ(wknight, (pos.get_pieces(Color::WHITE, PieceType::KNIGHT)));
    ASSERT_EQ(wking | wpawn | wknight, (pos.get_pieces(Color::WHITE)));
    ASSERT_EQ(bking | bpawn, (pos.get_pieces(Color::BLACK)));
    ASSERT_EQ(wking | bking, (pos.get_pieces(PieceType::KING)));
    ASSERT_EQ(wpawn | bpawn, (pos.get_pieces(PieceType::PAWN)));
    ASSERT_EQ(wknight, (pos.get_pieces(PieceType::KNIGHT)));
}

TEST(positiontest, test_promoting_pawns)
{
    const Position pos(
        Notation::to_position("Rr1n2RR/P2PP2P/8/2kp4/5KP1/8/p2pp2p/rR1N2rr w - - 0 1"));
    const U64 w_promotions = 0x11000000000000;
    const U64 b_promotions = 0x1100;

    const U64 w_res = pos.promoting_pawns(Color::WHITE);
    const U64 b_res = pos.promoting_pawns(Color::BLACK);

    ASSERT_EQ(w_promotions, w_res) << "Expected:\n"
                                   << Bitboard::pretty(w_promotions) << "\nGot:\n"
                                   << Bitboard::pretty(w_res);
    ASSERT_EQ(b_promotions, b_res) << "Expected:\n"
                                   << Bitboard::pretty(b_promotions) << "\nGot:\n"
                                   << Bitboard::pretty(b_res);
}
