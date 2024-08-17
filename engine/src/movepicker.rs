use chess::{Board, ChessMove, MoveGen, Piece};

use crate::chessmove::ChessMoveExt;
use crate::movelist::{MoveEntry, MoveVec};
use crate::newtypes::Value;
use crate::opts::OPTS;

/// Return an iterator over the legal moves on the board.
///
/// The moves are sorted in a way so that the expected best moves are first. This is done by
/// a combinations of heuristics and statistics gathered during search.
pub fn movepicker<const N: usize>(
    board: &Board,
    tt_move: Option<ChessMove>,
    killers: &[Option<ChessMove>; N],
    history_stats: &[[usize; 64]; 64],
    captures_only: bool,
) -> impl Iterator<Item = ChessMove> {
    let movegen = MoveGen::new_legal(board);
    let mut moves = Vec::with_capacity(movegen.len());

    let mut history_max = 1;
    for mv in movegen {
        let mut value = 0;

        let captures = mv.captures(board);
        let promotion = mv.get_promotion();

        if captures.is_some() || promotion.is_some() {
            value = 10 * i64::from(mvv_lva(mv, board));
        } else if captures.is_none() && captures_only {
            continue;
        }
        if killers.iter().any(|k| *k == Some(mv)) {
            value += i64::from(piece_value(Piece::Pawn) - 1);
        }

        let history_value = history_stats[mv.get_source().to_index()][mv.get_dest().to_index()];
        history_max = history_max.max(history_value);

        moves.push(MoveEntry {
            value: (value, history_value),
            mv,
        });
    }

    for entry in &mut moves {
        if Some(entry.mv) == tt_move {
            entry.value.0 = i64::MAX;
        } else {
            //entry.value.0 -= (entry.value.1 * OPTS.max_history_stats_impact / history_max) as i64;
            entry.value.0 += (entry.value.1 * 10 / history_max) as i64;
        }
        entry.value.0 = -entry.value.0;
        entry.value.1 = 0;
    }

    moves.sort_unstable();
    for entry in moves.iter().rev() {
        println!("{} {}", entry.mv, entry.value.0);
    }
    moves.into_iter().map(|m| m.mv)
}

/// Generate root moves.
///
/// The [`MoveVec`] is sorted by the same move picker logic as used during search, but of course
/// without any extra information like killer moves or tt moves. This mostly means that any winning
/// captures are first, and otherwise this doesn't matter much as root moves are sorted on each
/// depth increase.
pub fn root_moves(board: &Board) -> MoveVec<Value> {
    let empty_stats = [[0; 64]; 64];
    movepicker(board, None, &[], &empty_stats, false)
        .map(MoveEntry::new)
        .collect::<Vec<_>>()
        .into()
}

fn mvv_lva(mv: ChessMove, board: &Board) -> i32 {
    debug_assert!(
        mv.get_promotion().is_some() || mv.captures(board).is_some(),
        "MVV/LVA should only be done on captures and promotions, not {mv}, {:?}",
        board.en_passant()
    );
    let mut value = 0;

    // In case of promotions, add the value of the promoted piece to the move value.
    if let Some(piece) = mv.get_promotion() {
        value += piece_value(piece) - piece_value(Piece::Pawn);
    }

    // Capture? Prefer captures with higher value victims, by lower value attackers.
    if let Some(target) = mv.captures(board) {
        debug_assert!(
            board.piece_on(mv.get_source()).is_some(),
            "there should always be a source piece for a move"
        );
        value += piece_value(target)
            - piece_value(
                // SAFETY: The move is legal for the board, so there should always be a piece on
                // the source square.
                unsafe { board.piece_on(mv.get_source()).unwrap_unchecked() },
            );
    }

    value
}

/// Piece values for use with MVV/LVA only.
const fn piece_value(piece: Piece) -> i32 {
    match piece {
        Piece::Pawn => 100,
        Piece::Knight => 320,
        Piece::Bishop => 330,
        Piece::Rook => 500,
        Piece::Queen => 900,
        Piece::King => 950,
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashSet;

    use super::*;

    fn setup() -> (Board, HashSet<ChessMove>) {
        let board: Board = "r3k2r/p2pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 2"
            .parse()
            .unwrap();

        let expected_good_captures: HashSet<ChessMove> = ["g2h3", "d5e6", "d5c6", "e2a6"]
            .iter()
            .map(|s| s.parse::<ChessMove>().unwrap())
            .collect();

        (board, expected_good_captures)
    }

    #[test]
    fn test_expected_move_order_no_tt_or_killer() {
        let (board, expected_good_captures) = setup();
        let empty_stats = [[0; 64]; 64];
        let moves = movepicker(&board, None, &[], &empty_stats, false).collect::<Vec<_>>();
        for mv in &moves {
            println!("{mv} {:?}", mv.captures(&board));
        }
        assert_eq!(MoveGen::new_legal(&board).len(), moves.len());
        assert_eq!(
            expected_good_captures,
            moves.iter().take(4).copied().collect()
        );
        assert!(moves
            .iter()
            .skip(4)
            .take(40)
            .all(|mv| mv.captures(&board).is_none()));
        assert!(moves
            .iter()
            .skip(44)
            .all(|mv| mv.captures(&board).is_some()));
    }

    #[test]
    fn test_expected_move_order_tt_and_killer() {
        let (board, expected_good_captures) = setup();
        let empty_stats = [[0; 64]; 64];

        // Add a quiet move as the tt.
        let tt = "e1c1".parse::<ChessMove>().unwrap();
        // Try a random quiet, a None and the tt for killers.
        let killers = [Some("d2g5".parse().unwrap()), None, Some(tt)];
        let moves = movepicker(&board, Some(tt), &killers, &empty_stats, false).collect::<Vec<_>>();

        assert_eq!(MoveGen::new_legal(&board).len(), moves.len());

        let mut moves = moves.iter();
        assert_eq!(Some(tt), moves.next().copied());
        for _ in 0..4 {
            assert!(moves
                .next()
                .is_some_and(|mv| expected_good_captures.contains(mv)));
        }
        assert_eq!(killers[0], moves.next().copied());
        for _ in 0..38 {
            let mv = moves.next().unwrap();
            assert!(mv.captures(&board).is_none(), "{mv}");
        }

        for _ in 0..5 {
            let mv = moves.next().unwrap();
            assert!(mv.captures(&board).is_some());
        }
    }

    #[test]
    fn test_only_captures() {
        let (board, expected_good_captures) = setup();
        let empty_stats = [[0; 64]; 64];

        // Add a quiet move as the tt.
        let tt = "e1c1".parse::<ChessMove>().unwrap();
        // Try a random quiet, a None and the tt for killers.
        let killers = [Some("d2g5".parse().unwrap()), None, Some(tt)];
        let moves = movepicker(&board, Some(tt), &killers, &empty_stats, true).collect::<Vec<_>>();

        assert_eq!(9, moves.len());

        let mut moves = moves.iter();
        for _ in 0..4 {
            assert!(moves
                .next()
                .is_some_and(|mv| expected_good_captures.contains(mv)));
        }
        assert_eq!(5, moves.count());
    }
}
