use chess::{BitBoard, Board, Piece};

pub fn evaluate(board: &Board) -> i32 {
    let mut score = 0;

    let me = board.side_to_move();
    let them = !me;

    let my_pieces = board.color_combined(me);
    let their_pieces = board.color_combined(them);

    let pawns = board.pieces(Piece::Pawn);
    let knights = board.pieces(Piece::Knight);
    let bishops = board.pieces(Piece::Bishop);
    let rooks = board.pieces(Piece::Rook);
    let queens = board.pieces(Piece::Queen);

    let count = |color: &BitBoard, pieces: &BitBoard| (color & pieces).popcnt() as i32;

    score += count(my_pieces, pawns) * 100;
    score -= count(their_pieces, pawns) * 100;

    score += count(my_pieces, knights) * 320;
    score -= count(their_pieces, knights) * 320;

    let my_bishops = count(my_pieces, bishops);
    let their_bishops = count(their_pieces, bishops);
    score += if my_bishops >= 2 { 50 } else { 0 };
    score -= if their_bishops >= 2 { 50 } else { 0 };
    score += my_bishops * 330;
    score -= their_bishops * 330;

    score += count(my_pieces, rooks) * 500;
    score -= count(their_pieces, rooks) * 500;

    score += count(my_pieces, queens) * 900;
    score -= count(their_pieces, queens) * 900;

    score + 1 // Add 1 to account for tempo.
}
