/// Evaluate a `Board`.
///
/// The evaluation is based on the [Simplified Evaluation Function](cp-wiki).
/// It uses piece values and piece-square tables to evaluate the position.
/// The evaluation is very simple, but should be sufficient to play a decent game of chess.
///
/// [cp-wiki]: https://www.chessprogramming.org/Simplified_Evaluation_Function
use chess::{BitBoard, Board, Piece, Square};

use crate::value::Value;

#[rustfmt::skip]
const PAWN_TABLE: [i8; 64] = [
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
];
#[rustfmt::skip]
const KNIGHT_TABLE: [i8; 64] = [
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
];
#[rustfmt::skip]
const BISHOP_TABLE: [i8; 64] = [
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
];
#[rustfmt::skip]
const ROOK_TABLE: [i8; 64] = [
    0,  0,  0,  5,  5,  0,  0,  0,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    5, 10, 10, 10, 10, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
];
#[rustfmt::skip]
const QUEEN_TABLE: [i8; 64] = [
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
];
#[rustfmt::skip]
const KING_MIDDLEGAME_TABLE: [i8; 64] = [
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
];
#[rustfmt::skip]
const KING_ENDGAME_TABLE: [i8; 64] = [
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
];

pub trait Evaluate {
    fn evaluate(&self) -> Value;
}

impl Evaluate for Board {
    fn evaluate(&self) -> Value {
        evaluate(self)
    }
}

/// Evaluate a board.
///
/// The score is given w.r.t. the side to move, i.e. positive if the side to move has the better
/// position, and negative if the opponent has the better position.
fn evaluate(board: &Board) -> Value {
    let mut score: Value = 0;

    score += evaluate_material(board);
    score += evaluate_piece_square_tables(board);
    score += 1; // Add bonus for tempo.

    score
}

fn evaluate_material(board: &Board) -> Value {
    let mut score = 0;

    let my_pieces = board.color_combined(board.side_to_move());
    let their_pieces = board.color_combined(!board.side_to_move());

    let pawns = board.pieces(Piece::Pawn);
    let knights = board.pieces(Piece::Knight);
    let bishops = board.pieces(Piece::Bishop);
    let rooks = board.pieces(Piece::Rook);
    let queens = board.pieces(Piece::Queen);

    // The result of `popcnt` is always in the range 0..64, so it can be safely cast to `i32`.
    #[allow(clippy::cast_possible_wrap)]
    let count = |color: &BitBoard, pieces: &BitBoard| (color & pieces).popcnt() as i32;

    score += count(my_pieces, pawns) * 100;
    score -= count(their_pieces, pawns) * 100;

    score += count(my_pieces, knights) * 320;
    score -= count(their_pieces, knights) * 320;

    score += count(my_pieces, bishops) * 330;
    score -= count(their_pieces, bishops) * 330;

    score += count(my_pieces, rooks) * 500;
    score -= count(their_pieces, rooks) * 500;

    score += count(my_pieces, queens) * 900;
    score -= count(their_pieces, queens) * 900;

    score
        .try_into()
        .expect("material evaluation should not overflow an i16")
}

/// Evaluate the position using piece-square tables.
///
/// The score is given w.r.t. the side to move, i.e. positive if the side to move has the better
/// position, and negative if the opponent has the better position.
fn evaluate_piece_square_tables(board: &Board) -> Value {
    let mut total = 0;

    for s in (0..64).map(|s| unsafe {
        // Safety: `s` is always in the range 0..64.
        Square::new(s)
    }) {
        let piece = board.piece_on(s);
        if piece.is_none() {
            continue;
        }

        let piece = piece.unwrap();
        let color = board.color_on(s).unwrap();

        let table = match piece {
            Piece::Pawn => &PAWN_TABLE,
            Piece::Knight => &KNIGHT_TABLE,
            Piece::Bishop => &BISHOP_TABLE,
            Piece::Rook => &ROOK_TABLE,
            Piece::Queen => &QUEEN_TABLE,
            Piece::King if is_endgame(board) => &KING_ENDGAME_TABLE,
            Piece::King => &KING_MIDDLEGAME_TABLE,
        };

        let index = if color == chess::Color::White {
            s.to_index()
        } else {
            // Note that this doesn't "flip" the board, but rather rotates it 180 degrees.
            // The piece-square tables are symmetric, so this makes no difference, and is
            // simpler (and possibly faster) to implement.
            63 - s.to_index()
        };

        let score = i32::from(table[index]);

        if color == chess::Color::White {
            total += score;
        } else {
            total -= score;
        }
    }

    total
        .try_into()
        .expect("piece square evaluation should not overflow an i16")
}

/// Determine if a given board is in the endgame.
///
/// The endgame is defined as the board having one of the following conditions:
///
/// - No queens on the board.
/// - Every side which has a queen has additionally no other pieces or one minorpiece maximum.
fn is_endgame(board: &Board) -> bool {
    let my_pieces = board.color_combined(board.side_to_move());
    let their_pieces = board.color_combined(!board.side_to_move());
    let queens = board.pieces(Piece::Queen);

    let minor_pieces = board.pieces(Piece::Knight) | board.pieces(Piece::Bishop);
    let rooks = board.pieces(Piece::Rook);

    let definite_endgame = queens == &chess::EMPTY;
    let me_endgame = (my_pieces & queens) == chess::EMPTY
        || ((my_pieces & rooks) == chess::EMPTY && (my_pieces & minor_pieces).popcnt() <= 1);
    let them_endgame = (their_pieces & queens) == chess::EMPTY
        || ((their_pieces & rooks) == chess::EMPTY && (their_pieces & minor_pieces).popcnt() <= 1);

    definite_endgame || (me_endgame && them_endgame)
}
