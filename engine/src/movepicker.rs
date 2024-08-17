use std::mem::MaybeUninit;

use chess::{Board, ChessMove, MoveGen, Piece};

use crate::chessmove::ChessMoveExt;
use crate::movelist::{MoveEntry, MoveVec};

pub struct MovePicker<const N: usize> {
    board: Board,
    stage: Stage,
    tt_move: Option<ChessMove>,
    killers: [Option<ChessMove>; N],
    move_ordering: MaybeUninit<MoveOrdering>,
    captures_only: MovePickerKind,
}

type MovePickerKind = bool;
pub const ALL_MOVES: MovePickerKind = false;
pub const CAPTURES_ONLY: MovePickerKind = true;
type MoveIdx = usize;

enum Stage {
    HashMove,
    GoodCaptures(MoveIdx),
    Killer(MoveIdx),
    Quiet(MoveIdx),
    BadCaptures(MoveIdx),
}

struct MoveOrdering {
    moves: MoveVec<i32>,
    n_good_captures_or_promotions: usize,
    n_bad_captures: usize,
    n_quiets: usize,
}

impl<const N: usize> MovePicker<N> {
    pub fn new(
        captures_only: MovePickerKind,
        board: Board,
        tt_move: Option<ChessMove>,
        killers: [Option<ChessMove>; N],
    ) -> Self {
        if let Some(mv) = tt_move {
            if board.legal(mv) && (!captures_only || mv.captures(&board).is_some()) {
                return Self {
                    board,
                    stage: Stage::HashMove,
                    tt_move: Some(mv),
                    killers,
                    move_ordering: MaybeUninit::uninit(),
                    captures_only,
                };
            }
        }

        let stage = Stage::GoodCaptures(0);
        Self {
            board,
            stage,
            tt_move: None,
            killers,
            move_ordering: MaybeUninit::uninit(),
            captures_only,
        }
    }
}

impl<const N: usize> Iterator for MovePicker<N> {
    type Item = ChessMove;

    fn next(&mut self) -> Option<Self::Item> {
        match self.stage {
            Stage::HashMove => {
                self.stage = Stage::GoodCaptures(0);
                self.tt_move
            }
            Stage::GoodCaptures(idx) => {
                if idx == 0 {
                    self.move_ordering
                        .write(MoveOrdering::new(&self.board, self.tt_move));
                }

                // SAFETY: We just initialized the `move_ordering` field, as idx!=0 happens
                // strictly after the first iteration.
                let mvo = unsafe { self.move_ordering.assume_init_ref() };

                if idx >= mvo.n_good_captures_or_promotions {
                    self.stage = if self.captures_only {
                        Stage::BadCaptures(0)
                    } else {
                        Stage::Killer(0)
                    };
                    return self.next();
                }

                let mv = mvo.moves[idx].mv;
                self.stage = Stage::GoodCaptures(idx + 1);
                Some(mv)
            }
            Stage::Killer(idx) => {
                // SAFETY: The `move_ordering` field is initialized before this stage.
                let mvo = unsafe { self.move_ordering.assume_init_mut() };

                if let Some(killer) = self.killers.get(idx) {
                    self.stage = Stage::Killer(idx + 1);
                    if killer.is_some_and(|killer| mvo.pop_killer(killer)) {
                        return *killer;
                    }
                    return self.next();
                }

                mvo.order_quiets();
                self.stage = Stage::Quiet(0);
                self.next()
            }
            Stage::Quiet(idx) => {
                // SAFETY: The `move_ordering` field is initialized before this stage.
                let mvo = unsafe { self.move_ordering.assume_init_ref() };
                if idx >= mvo.n_quiets {
                    self.stage = Stage::BadCaptures(0);
                    return self.next();
                }
                self.stage = Stage::Quiet(idx + 1);
                Some(mvo.moves[mvo.n_good_captures_or_promotions + mvo.n_bad_captures + idx].mv)
            }
            Stage::BadCaptures(idx) => {
                // SAFETY: The `move_ordering` field is initialized before this stage.
                let mvo = unsafe { self.move_ordering.assume_init_ref() };
                if idx >= mvo.n_bad_captures {
                    return None;
                }

                let mv = mvo.moves[mvo.n_good_captures_or_promotions + idx].mv;
                self.stage = Stage::BadCaptures(idx + 1);
                Some(mv)
            }
        }
    }
}

impl MoveOrdering {
    fn new(board: &Board, ignore: Option<ChessMove>) -> Self {
        let mut movegen = MoveGen::new_legal(board);
        let mut moves = Vec::with_capacity(movegen.len());

        // Ignore the tt move, if any, as this has already been iterated before this is called.
        if let Some(mv) = ignore {
            movegen.remove_move(mv);
        }

        // Iterate the captures first (ignoring ep at this point because it cannot be included with a mask).
        movegen.set_iterator_mask(*board.color_combined(!board.side_to_move()));
        for mv in &mut movegen {
            moves.push(MoveEntry {
                mv,
                value: mvv_lva(mv, board),
            });
        }
        let mut n_cap_promos = moves.len();

        // And now all the quiets.
        // Non-capturing promotions are included here, but we put them with the captures as they
        // change the material count (and are likely good moves). In the case of a promotion, we
        // place it last among the captures (which will be sorted later).
        // Also, en passant is included here.
        movegen.set_iterator_mask(!chess::EMPTY);
        for mv in movegen {
            moves.push(MoveEntry { mv, value: 0 });

            // Because the only capture that appears here, we simply just test for a promotion or capture.
            if mv.get_promotion().is_some() || mv.captures(board).is_some() {
                debug_assert!(n_cap_promos < moves.len());
                let last = moves.len() - 1;
                moves[last].value = mvv_lva(mv, board);
                moves.swap(last, n_cap_promos);
                n_cap_promos += 1;
            }
        }

        // To start, sort the captures into winning/equal captures and losing captures.
        let (captures, _) = moves.split_at_mut(n_cap_promos);
        captures.sort_by_key(|m| -m.value);

        let n_good_captures_or_promotions = captures
            .iter()
            .position(|m| m.value < 0)
            .unwrap_or(captures.len());
        let n_bad_captures = n_cap_promos - n_good_captures_or_promotions;
        let n_quiets = moves.len() - n_cap_promos;

        Self {
            moves: moves.into(),
            n_good_captures_or_promotions,
            n_bad_captures,
            n_quiets,
        }
    }

    fn pop_killer(&mut self, killer: ChessMove) -> bool {
        let quiet_start = self.n_good_captures_or_promotions + self.n_bad_captures;
        if let Some(idx) = self.moves[quiet_start..]
            .iter()
            .position(|entry| entry.mv == killer)
        {
            self.moves.swap_remove(quiet_start + idx);
            self.n_quiets -= 1;
            return true;
        }
        false
    }

    #[allow(clippy::unused_self)]
    fn order_quiets(&mut self) {
        // TODO: Score quiet moves somehow.
        //
        // let _quiet_start = self.n_good_captures_or_promotions + self.n_bad_captures;
        // let (_, quiets) = self.moves.split_at_mut(quiet_start);
        // quiets.sort_by_key(|m| -m.value);
    }
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
        let moves = MovePicker::new(false, board, None, []).collect::<Vec<_>>();
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

        // Add a quiet move as the tt.
        let tt = "e1c1".parse::<ChessMove>().unwrap();
        // Try a random quiet, a None and the tt for killers.
        let killers = [Some("d2g5".parse().unwrap()), None, Some(tt)];
        let moves = MovePicker::new(ALL_MOVES, board, Some(tt), killers).collect::<Vec<_>>();

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

        // Add a quiet move as the tt.
        let tt = "e1c1".parse::<ChessMove>().unwrap();
        // Try a random quiet, a None and the tt for killers.
        let killers = [Some("d2g5".parse().unwrap()), None, Some(tt)];
        let moves = MovePicker::new(CAPTURES_ONLY, board, Some(tt), killers).collect::<Vec<_>>();

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
