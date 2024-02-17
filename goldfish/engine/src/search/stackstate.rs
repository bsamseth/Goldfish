use chess::ChessMove;

#[derive(Debug, Clone, Copy, Default)]
pub struct StackState {
    pub killers: [Option<ChessMove>; 2],
    pub halfmove_clock: usize,
    pub zobrist: u64,
}

impl StackState {
    pub fn update_killer(&mut self, mv: Option<ChessMove>) {
        if mv.is_some() && mv != self.killers[0] {
            self.killers[1] = self.killers[0];
            self.killers[0] = mv;
        }
    }
}
