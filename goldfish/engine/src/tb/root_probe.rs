use super::{Tablebase, Wdl};
use crate::board::BoardExt;
use crate::movelist::MoveVec;
use crate::newtypes::Value;

#[derive(Debug)]
pub struct RootMovesFilter<'a> {
    wdl: Wdl,
    tb: &'a Tablebase,
    board: &'a chess::Board,
    halfmove_clock: usize,
}

impl<'a> RootMovesFilter<'a> {
    pub fn new(
        wdl: Wdl,
        tb: &'a Tablebase,
        board: &'a chess::Board,
        halfmove_clock: usize,
    ) -> Self {
        Self {
            wdl,
            tb,
            board,
            halfmove_clock,
        }
    }

    /// Filter the provided `moves` to only include moves that preserve the [`Wdl`] value.
    pub fn filter_moves(&self, moves: &mut MoveVec) {
        let wdl_opponent = match self.wdl {
            Wdl::Win => Wdl::Loss,
            Wdl::Loss => Wdl::Win,
            Wdl::Draw => Wdl::Draw,
            Wdl::BlessedLoss => Wdl::CursedWin,
            Wdl::CursedWin => Wdl::BlessedLoss,
        };
        moves.retain(|mv| {
            let halfmove_clock = if self.board.halfmove_reset(mv.mv) {
                0
            } else {
                self.halfmove_clock + 1
            };
            let next = self.board.make_move_new(mv.mv);
            self.tb
                .probe_wdl(&next, halfmove_clock)
                .map_or(false, |wdl| wdl == wdl_opponent)
        });
    }

    /// Get the [`Value`] of the root position.
    ///
    /// This is the value of the position from the perspective of the side to move,
    /// counting all kinds of draws as `Value::DRAW`.
    pub fn score(&self) -> Value {
        match self.wdl {
            Wdl::Win => Value::KNOWN_WIN,
            Wdl::Loss => -Value::KNOWN_WIN,
            Wdl::Draw | Wdl::BlessedLoss | Wdl::CursedWin => Value::DRAW,
        }
    }
}
