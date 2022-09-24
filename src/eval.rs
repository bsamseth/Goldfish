use chess::{BitBoard, Board, Color, Piece};
use vampirc_uci::UciInfoAttribute;

/// Mate evaluation. 
/// Larger than any sensible evaluation, but also leaves room to not overflow.
pub const VALUE_MATE: i32 = i32::MAX - 1 - (u8::MAX as i32);
pub const VALUE_MATE_IN_MAX_PLY: i32 = VALUE_MATE - (u8::MAX as i32);

pub fn score_as_uci_info(score: i32) -> UciInfoAttribute {
    if score.abs() < VALUE_MATE_IN_MAX_PLY {
        UciInfoAttribute::from_centipawns(score)
    } else {
        let mate_in = ((VALUE_MATE - score.abs()) / 2) as i8;
        if score > 0 {
            UciInfoAttribute::from_mate(mate_in)
        } else {
            UciInfoAttribute::from_mate(-mate_in)
        }
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

    score += piece_count_diff(&white_pieces, &black_pieces, &queens) * 900;
    score += piece_count_diff(&white_pieces, &black_pieces, &rooks) * 500;
    score += piece_count_diff(&white_pieces, &black_pieces, &bishops) * 310;
    score += piece_count_diff(&white_pieces, &black_pieces, &knights) * 300;
    score += piece_count_diff(&white_pieces, &black_pieces, &pawns) * 100;

    if board.side_to_move() == Color::White {
        score
    } else {
        -score
    }
}
