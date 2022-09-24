use chess::{BitBoard, Board, Color, Piece};
use vampirc_uci::UciInfoAttribute;

/// Mate evaluation.
/// Larger than any sensible evaluation, but also leaves room to not overflow.
pub const VALUE_MATE: i32 = i32::MAX - 1 - (u8::MAX as i32);

/// Mate in the maximum number of plies recongnized by the engine.
pub const VALUE_MATE_IN_MAX_PLY: i32 = VALUE_MATE - (u8::MAX as i32);

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

/// Return how many more white pieces there are than black pieces.
fn piece_count_diff(whites: &BitBoard, blacks: &BitBoard, pieces: &BitBoard) -> i32 {
    let white_count = (pieces & whites).popcnt() as i32;
    let black_count = (pieces & blacks).popcnt() as i32;
    white_count - black_count
}

/// Evaluate the given board, returning a centipawn score.
///
/// The score is returned with respect to the moving color.
pub fn eval(board: &Board) -> i32 {
    let mut score = 0;

    let white_pieces = board.color_combined(Color::White);
    let black_pieces = board.color_combined(Color::Black);
    let queens = board.pieces(Piece::Queen);
    let rooks = board.pieces(Piece::Rook);
    let bishops = board.pieces(Piece::Bishop);
    let knights = board.pieces(Piece::Knight);
    let pawns = board.pieces(Piece::Pawn);

    score += piece_count_diff(white_pieces, black_pieces, queens) * 900;
    score += piece_count_diff(white_pieces, black_pieces, rooks) * 500;
    score += piece_count_diff(white_pieces, black_pieces, bishops) * 310;
    score += piece_count_diff(white_pieces, black_pieces, knights) * 300;
    score += piece_count_diff(white_pieces, black_pieces, pawns) * 100;

    if board.side_to_move() == Color::White {
        score
    } else {
        -score
    }
}
