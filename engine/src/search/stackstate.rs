use chess::ChessMove;

use crate::newtypes::Value;

#[derive(Debug, Clone, Copy)]
pub struct StackState {
    pub eval: Value,
    pub killers: [Option<ChessMove>; 2],
    pub halfmove_clock: usize,
    pub zobrist: u64,
    pub null_move: bool,
}

impl Default for StackState {
    fn default() -> Self {
        Self {
            eval: -Value::INFINITE,
            null_move: false,
            killers: Default::default(),
            halfmove_clock: Default::default(),
            zobrist: Default::default(),
        }
    }
}

impl StackState {
    /// Update the killer moves of the [`StackState`].
    ///
    /// The previous first killer move is moved to the second slot, and the new move is placed in the
    /// first slot. If the new move is the same as the previous first killer move, nothing is changed.
    pub fn update_killer(&mut self, mv: Option<ChessMove>) {
        if mv.is_some() && mv != self.killers[0] {
            self.killers[1] = self.killers[0];
            self.killers[0] = mv;
        }
    }
}
