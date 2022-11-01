use chess::{Board, Color, Piece, ALL_SQUARES};
use vampirc_uci::UciInfoAttribute;

mod options;

use crate::eval::options::*;

/// Mate evaluation.
/// Larger than any sensible evaluation, but also leaves room to not overflow.
const VALUE_MATE: i32 = i32::MAX - 1 - (u8::MAX as i32);

/// Mate in the maximum number of plies recognized by the engine.
const VALUE_MATE_IN_MAX_PLY: i32 = VALUE_MATE - (u8::MAX as i32);

/// True if the score is a mate score.
///
/// # Examples
/// ```
/// use goldfish::eval;
/// assert!(eval::is_mate(eval::mate_in_ply(4)));
/// assert!(!eval::is_mate(123));
/// ```
pub const fn is_mate(value: i32) -> bool {
    value.abs() >= VALUE_MATE_IN_MAX_PLY
}

/// Produce a nominal score representing a mate in `ply` half-moves.
///
/// # Examples
/// ```
/// use goldfish::eval;
/// let score = eval::mate_in_ply(3);
/// assert!(eval::is_mate(score));
/// assert_eq!(score, eval::VALUE_MATE - 3);
/// assert_eq!(eval::mate_distance(score), 3);
/// ```
pub const fn mate_in_ply(ply: u8) -> i32 {
    VALUE_MATE - (ply as i32)
}

/// Given a mate score, return the distance to mate in half-moves.
///
/// If the score is not a mate score, the value will not be meaningful.
///
/// # Examples
/// ```
/// use goldfish::eval;
/// let score = eval::mate_in_ply(8);
/// let negative_score = -eval::mate_in_ply(11);
/// assert_eq!(eval::mate_distance(score), 8);
/// assert_eq!(eval::mate_distance(negative_score), 11);
/// ```
pub const fn mate_distance(value: i32) -> u8 {
    debug_assert!(is_mate(value));
    (VALUE_MATE - value.abs()) as u8
}

/// Return the score from the perspective of the white player.
///
/// This assumes that the score is from the perspective of the side to move.
///
/// # Examples
/// ```
/// use goldfish::eval;
/// use chess::Color;
/// let score = 10;
/// assert_eq!(eval::normalize_score(score, Color::White), 10);
/// assert_eq!(eval::normalize_score(score, Color::Black), -10);
/// ```
pub fn normalize_score(score: i32, side_to_move: chess::Color) -> i32 {
    if side_to_move == chess::Color::White {
        score
    } else {
        -score
    }
}

/// Return the score as a `UciInfoAttribute`.
///
/// This handles formatting mate scores as "mate in x", as well as normal centipawn scores.
pub fn score_as_uci_info(score: i32) -> UciInfoAttribute {
    if is_mate(score) {
        let mate_in = ((VALUE_MATE - score.abs() + 1) / 2) as i8;
        if score > 0 {
            UciInfoAttribute::from_mate(mate_in)
        } else {
            UciInfoAttribute::from_mate(-mate_in)
        }
    } else {
        UciInfoAttribute::from_centipawns(score)
    }
}

/// Evaluate the given board, returning a centipawn score.
///
/// The score is returned with respect to the moving color.
pub fn eval(board: &Board) -> i32 {
    eval_material(board)
}

fn eval_material(board: &Board) -> i32 {
    let mut score = 0;
    for square in ALL_SQUARES {
        if let Some(piece) = board.piece_on(square) {
            let color = board.color_on(square).unwrap();
            let piece_value = piece_value(piece, color, square);
            if color == Color::White {
                score += piece_value;
            } else {
                score -= piece_value;
            }
        }
    }

    if board.side_to_move() == Color::White {
        score
    } else {
        -score
    }
}

fn piece_value(piece: Piece, color: Color, square: chess::Square) -> i32 {
    let piece_value = match piece {
        Piece::Pawn => PAWN_VALUE,
        Piece::Knight => KNIGHT_VALUE,
        Piece::Bishop => BISHOP_VALUE,
        Piece::Rook => ROOK_VALUE,
        Piece::Queen => QUEEN_VALUE,
        Piece::King => 0,
    };

    piece_value + piece_square_value(piece, color, square)
}

fn piece_square_value(piece: Piece, color: Color, square: chess::Square) -> i32 {
    let piece_square_table = match piece {
        Piece::Pawn => PAWN_SQUARE_TABLE,
        Piece::Knight => KNIGHT_SQUARE_TABLE,
        Piece::Bishop => BISHOP_SQUARE_TABLE,
        Piece::Rook => ROOK_SQUARE_TABLE,
        Piece::Queen => QUEEN_SQUARE_TABLE,
        Piece::King => KING_SQUARE_TABLE,
    };

    let index = match color {
        Color::White => square.to_index(),
        Color::Black => 63 - square.to_index(),
    };

    piece_square_table[index]
}
